#include "Runtime/Concurrency/ConcurrencyModule.h"
#include "Runtime/Concurrency/ConcurrentContainers.h"
#include "Runtime/Concurrency/ConcurrentDefs.h"
#include "Runtime/Concurrency/SpinLock.h"
#include "Runtime/Concurrency/ThreadUtils.h"
#include "Runtime/Memory/MemoryModule.h"
#include "Runtime/Misc/Padding.h"
#include "Runtime/Misc/PImplUtils.h"
#include "Runtime/Misc/PMRContainers.h"
#include "Runtime/System/Module.h"
#include "Runtime/System/ModuleExport.h"
#include "Runtime/System/ModuleImplHelpers.h"

#if OMNI_CLANG
#else
#include <Windows.h>
#endif
#include <thread>

namespace Omni
{
    //forard decalrations
    struct ConcurrencyModulePrivateImpl
    {
    public:
        DispatchQueue                   mSerialQueues[(u32)QueueKind::Max];
        ConcurrentQueue                 mSharedQueue;
        ThreadData**                    mThreadData;
        u32                             mThreadCount;
    public:
        ConcurrencyModulePrivateImpl();
        void WaitWorkersQuitOnMain();
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

        //1. init serial queues
        const char* queueNames[] = {
#define QUEUE_KIND(x) #x,
#include "Runtime/Concurrency/QueueKind.inl"
#undef QUEUE_KIND
        };
        for (u32 iQueue = 0; iQueue < (u32)QueueKind::Max; ++iQueue)
        {
            self->mSerialQueues[iQueue].SetName(queueNames[iQueue]);
        }

        //3. worker threads
        u32 nThreads = (u32)std::thread::hardware_concurrency();
        //nThreads = 1;
        self->mThreadCount = nThreads;
        self->mThreadData = (ThreadData**)OmniMalloc(MemoryKind::SystemInit, sizeof(void*) * nThreads);
        self->mSharedQueue.SetAllWakeupLimit(nThreads / 2);
        for (u32 iThread = MainThreadIndex; iThread < nThreads; ++iThread)
            self->mThreadData[iThread] = &ThreadData::Create();
        self->mThreadData[0]->InitAsMainOnMain();
        for (u32 iThread = MainThreadIndex + 1; iThread < nThreads; ++iThread)
            self->mThreadData[iThread]->LauchAsWorkerOnMain();

        Module::Initialize(args);
    }
    void ConcurrencyModule::Finalize()
    {
        Module::Finalizing();
        if (GetUserCount() > 0)
            return;

        //3. worker threads
        ConcurrencyModuleImpl* self = ConcurrencyModuleImpl::GetCombinePtr(this);
        self->mThreadData[0]->JoinAndDestroyOnMain();
        OmniFree(MemoryKind::SystemInit, sizeof(void*) * self->mThreadCount, self->mThreadData);

        gConcurrencyModule = nullptr;
        Module::Finalize();
        MemoryModule::Get().Release();
    }
    void ConcurrencyModule::Finalizing()
    {
        Finalize();
    }
    ConcurrencyModule& ConcurrencyModule::Get()
    {
        return *gConcurrencyModule;
    }
    u32 ConcurrencyModule::GetWorkerCount()
    {
        ConcurrencyModuleImpl* self = ConcurrencyModuleImpl::GetCombinePtr(this);
        return (u32)self->mThreadCount;
    }
    DispatchQueue& ConcurrencyModule::GetQueue(QueueKind queueKind)
    {
        ConcurrencyModuleImpl* self = ConcurrencyModuleImpl::GetCombinePtr(this);
        return self->mSerialQueues[(u32)queueKind];
    }
    void ConcurrencyModule::Async(DispatchWorkItem& head)
    {
        DispatchWorkItem* p = &head;
        DispatchWorkItem* t = nullptr;
        while (p)
        {
            t = p;
            p = static_cast<DispatchWorkItem*>(p->Next);
        }
        ConcurrencyModuleImpl* self = ConcurrencyModuleImpl::GetCombinePtr(this);
        self->mSharedQueue.Enqueue(&head, t);
    }
    void ConcurrencyModule::DismissWorkers()
    {
        ConcurrencyModuleImpl* self = ConcurrencyModuleImpl::GetCombinePtr(this);
        DispatchWorkItem* lastJob = nullptr;
        for (u32 iThread = 0; iThread < self->mThreadCount; ++iThread)
        {
            DispatchWorkItem& item = DispatchWorkItem::Create(ThreadData::MarkQuitWork);
            item.Next = lastJob;
            lastJob = &item;
        }
        Async(*lastJob);
    }
    ConcurrencyModulePrivateImpl::ConcurrencyModulePrivateImpl()
        : mThreadData(nullptr)
        , mThreadCount(0)
    {
    }
    void ConcurrencyModulePrivateImpl::WaitWorkersQuitOnMain()
    {
        for (u32 iThread = MainThreadIndex + 1; iThread < (u32)mThreadCount; ++iThread)
            mThreadData[iThread]->JoinAndDestroyOnMain();
    }
    void ConcurrentWorkerThreadFunc(ThreadData* self);
    void ConcurrentWorkerThreadFunc(ThreadData* self)
    {
        ConcurrencyModuleImpl* cm = (ConcurrencyModuleImpl*)gConcurrencyModule;
        while (!self->IsAskedToQuit())
        {
            DispatchWorkItem* item = cm->mSharedQueue.Dequeue<DispatchWorkItem>();
            item->Perform();
            item->Destroy();
        }
        if (IsOnMainThread())
        {//MainThread then cleans up everything else
            //but first need to block it self until all other workers had received quit message
            cm->WaitWorkersQuitOnMain();
            bool todo = false;
            do
            {
                bool sharedQueueEmpty = cm->mSharedQueue.Size() == 0;
                if (!sharedQueueEmpty)
                {
                    DispatchWorkItem* item = cm->mSharedQueue.Dequeue<DispatchWorkItem>();
                    item->Perform();
                    item->Destroy();
                }
                //empty serial queues here
                todo = !sharedQueueEmpty;
            } while(todo);
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
    ExportInternalModule(Concurrency, ModuleExportInfo(ConcurrencyModuleCtor, true));
}
