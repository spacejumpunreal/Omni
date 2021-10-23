#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/Concurrency/ConcurrencyModule.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Base/Misc/Padding.h"
#include "Runtime/Base/MultiThread/LockQueue.h"
#include "Runtime/Base/MultiThread/SpinLock.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Concurrency/ConcurrentDefs.h"
#include "Runtime/Core/Concurrency/SingleConsumerQueues.h"
#include "Runtime/Core/Concurrency/ThreadUtils.h"
#include "Runtime/Core/System/Module.h"
#include "Runtime/Core/System/ModuleExport.h"
#include "Runtime/Core/System/ModuleImplHelpers.h"

#if OMNI_CLANG
#else
#include <Windows.h>
#endif
#include <thread>

namespace Omni
{
    using ThreadDataMap = PMRUnorderedMap<ThreadId, ThreadData*, std::hash<ThreadId>, std::equal_to<ThreadId>>;
    void ConcurrentWorkerThreadFunc(ThreadData* self);

    struct ConcurrencyModulePrivateImpl
    {
    public:
        LockQueue                   mQueues[(u32)QueueKind::Count];
        ThreadDataMap               mThreadData;
        u32                         mWorkerCount;
        ThreadId                    mNextThreadId;
    public:
        ConcurrencyModulePrivateImpl();
    };

    //definitions
    using ConcurrencyModuleImpl = PImplCombine<ConcurrencyModule, ConcurrencyModulePrivateImpl>;

    //global variables
    ConcurrencyModuleImpl* gConcurrencyModule;

    void ConcurrencyModule::Initialize(const EngineInitArgMap& args)
    {
        MemoryModule::Get().Retain();

        ConcurrencyModuleImpl* self = ConcurrencyModuleImpl::GetCombinePtr(this);
        CheckAlways(gConcurrencyModule == nullptr);
        gConcurrencyModule = self;

        //0. misc

        //1. main thread init
        {
            ThreadData& mainThreadData = ThreadData::Create(MainThreadId);
            self->mThreadData.emplace(std::make_pair(MainThreadId, &mainThreadData));
            mainThreadData.InitAsMainOnMain(); //main thread
        }

        //2. worker threads
        self->mWorkerCount = (u32)std::thread::hardware_concurrency();
        i32 offset = -2; //main thread and render thread
        auto it = args.find("--worker-thread-count");
        if (it != args.end())
        {
            offset = atoi(it->second.data());
            if (offset >= 0)
            {
                offset = 0;
                self->mWorkerCount = offset;
            }
        }
        self->mWorkerCount += offset;
        self->mQueues[(u32)QueueKind::Shared].SetAllWakeupLimit(self->mWorkerCount / 2);
        for (u32 iThread = 0; iThread < self->mWorkerCount; ++iThread)
        {
            ThreadId tid = WorkerThreadBaseId + iThread;
            ThreadData& threadData = ThreadData::Create(tid);
            self->mThreadData.emplace(std::make_pair(tid, &threadData));
            threadData.LauchAsWorkerOnMain(TThreadBody([&threadData] {
                ConcurrentWorkerThreadFunc(&threadData);
                }));
        }

        Module::Initialize(args);
    }

    void ConcurrencyModule::StopThreads()
    {
        ConcurrencyModuleImpl* self = ConcurrencyModuleImpl::GetCombinePtr(this);
        for (ThreadId wid = 0; wid < self->mWorkerCount; ++wid)
        {
            self->mThreadData[wid]->JoinAndDestroyOnMain();
            self->mThreadData[wid] = nullptr;
        }
    }

    void ConcurrencyModule::Finalize()
    {
        Module::Finalizing();
        if (GetUserCount() > 0)
            return;

        //2. worker threads
        ConcurrencyModuleImpl* self = ConcurrencyModuleImpl::GetCombinePtr(this);
        for (auto it = self->mThreadData.begin(); it != self->mThreadData.end(); ++it)
        {
            CheckAlways(it->second == nullptr);
        }
        self->mThreadData[MainThreadId]->JoinAndDestroyOnMain();

        gConcurrencyModule = nullptr;
        Module::Finalize();
        MemoryModule::Get().Release();
    }

    ConcurrencyModule& ConcurrencyModule::Get()
    {
        return *gConcurrencyModule;
    }

    u32 ConcurrencyModule::GetWorkerCount()
    {
        ConcurrencyModuleImpl* self = ConcurrencyModuleImpl::GetCombinePtr(this);
        return self->mWorkerCount;
    }

    LockQueue* ConcurrencyModule::GetQueue(QueueKind queueKind)
    {
        ConcurrencyModuleImpl* self = ConcurrencyModuleImpl::GetCombinePtr(this);
        return &self->mQueues[(u32)queueKind];
    }

    ThreadData* ConcurrencyModule::CreateThread(const TThreadBody& body, ThreadId designatedTid)
    {
        CheckAlways(IsOnMainThread());
        ConcurrencyModuleImpl* self = ConcurrencyModuleImpl::GetCombinePtr(this);
        ThreadId tid = designatedTid == InvalidThreadId ? self->mNextThreadId++ : designatedTid;
        ThreadData* ret = &ThreadData::Create(tid);
        self->mThreadData.emplace(std::make_pair(tid, ret));
        ret->LauchAsWorkerOnMain(body);
        return ret;
    }

    void ConcurrencyModule::EnqueueWork(DispatchWorkItem& head, QueueKind queueKind)
    {
        DispatchWorkItem* p = &head;
        DispatchWorkItem* t = nullptr;
        while (p)
        {
            t = p;
            p = static_cast<DispatchWorkItem*>(p->Next);
        }
        ConcurrencyModuleImpl* self = ConcurrencyModuleImpl::GetCombinePtr(this);
        self->mQueues[(u32)queueKind].Enqueue(&head, t);
    }

    void ConcurrencyModule::SignalWorkersToQuit()
    {
        ConcurrencyModuleImpl* self = ConcurrencyModuleImpl::GetCombinePtr(this);
        DispatchWorkItem* lastJob = nullptr;
        for (u32 iThread = 0; iThread < self->mWorkerCount; ++iThread)
        {
            DispatchWorkItem& item = DispatchWorkItem::Create(ThreadData::MarkQuitWork, MemoryKind::CacheLine);
            item.Next = lastJob;
            lastJob = &item;
        }
        if (lastJob != nullptr)
            self->EnqueueWork(*lastJob, QueueKind::Shared);
    }

    ConcurrencyModulePrivateImpl::ConcurrencyModulePrivateImpl()
        : mThreadData(MemoryModule::Get().GetPMRAllocator(MemoryKind::SystemInit))
        , mWorkerCount(0)
        , mNextThreadId(DynamicThreadBaseId)
    {
    }
    
    void ConcurrentWorkerThreadFunc(ThreadData* self)
    {
        ConcurrencyModuleImpl* cm = (ConcurrencyModuleImpl*)gConcurrencyModule;
        LockQueue& sharedQueue = cm->mQueues[(u32)QueueKind::Shared];
        while (!self->IsAskedToQuit())
        {
            DispatchWorkItem* item = sharedQueue.Dequeue<DispatchWorkItem>();
            item->Perform();
            item->Destroy();
        }
    }

    static Module* ConcurrencyModuleCtor(const EngineInitArgMap&)
    {
        return InitMemFactory<ConcurrencyModuleImpl>::New();
    }

    void ConcurrencyModule::Destroy()
    {
        InitMemFactory<ConcurrencyModuleImpl>::Delete((ConcurrencyModuleImpl*)this);
    }

    EXPORT_INTERNAL_MODULE(Concurrency, ModuleExportInfo(ConcurrencyModuleCtor, true));
}
