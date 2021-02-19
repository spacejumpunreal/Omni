#include "Runtime/Concurrency/ConcurrencyModule.h"
#include "Runtime/Concurrency/ConcurrentContainers.h"
#include "Runtime/Concurrency/ConcurrentDefs.h"
#include "Runtime/Concurrency/SpinLock.h"
#include "Runtime/Concurrency/ThreadUtils.h"
#include "Runtime/Memory/MemoryModule.h"
#include "Runtime/Misc/Padding.h"
#include "Runtime/Misc/PImplUtils.h"
#include "Runtime/Misc/PMRContainers.h"
#include "Runtime/System/ModuleExport.h"
#include "Runtime/System/Module.h"

#include <Windows.h>
namespace Omni
{
    //forard decalrations
    struct ConcurrencyModulePrivateImpl
    {
    public:
        DispatchQueue                   mSerialQueues[(u32)QueueKind::Max];
        ConcurrentQueue                 mSharedQueue;
        PMRVector<ThreadData*>   mThreadData;
    public:
        void WaitWorkersQuitOnMain();
    };

    //definitions
    using ConcurrencyModuleImpl = PImplCombine<ConcurrencyModule, ConcurrencyModulePrivateImpl>;

    //global variables
    ConcurrencyModule* gConcurrencyModule;

    void ConcurrencyModule::Initialize()
    {
        MemoryModule::Get().Retain();

        ConcurrencyModuleImpl* self = ConcurrencyModuleImpl::GetCombinePtr(this);
        gConcurrencyModule = this;

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
        Module::Initialize();

        //2. worker threads
        u32 nThreads = (u32)std::thread::hardware_concurrency();
        //nThreads = 1;
        self->mThreadData.resize(nThreads);
        self->mSharedQueue.SetAllWakeupLimit(nThreads / 2);
        for (u32 iThread = MainThreadIndex; iThread < nThreads; ++iThread)
            self->mThreadData[iThread] = &ThreadData::Create();
        self->mThreadData[0]->InitAsMainOnMain();
        for (u32 iThread = MainThreadIndex + 1; iThread < nThreads; ++iThread)
            self->mThreadData[iThread]->LauchAsWorkerOnMain();
    }
    void ConcurrencyModule::Finalize()
    {
        Module::Finalizing();
        if (GetUserCount() > 0)
            return;

        ConcurrencyModuleImpl* self = ConcurrencyModuleImpl::GetCombinePtr(this);
        self->mThreadData[0]->JoinAndDestroyOnMain();
        self->mThreadData.resize(0);
        self->mThreadData.shrink_to_fit();
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
        return (u32)self->mThreadData.size();
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
        for (u32 iThread = 0; iThread < (u32)self->mThreadData.size(); ++iThread)
        {
            DispatchWorkItem& item = DispatchWorkItem::Create(ThreadData::MarkQuitWork);
            item.Next = lastJob;
            lastJob = &item;
        }
        Async(*lastJob);
    }
    void ConcurrencyModulePrivateImpl::WaitWorkersQuitOnMain()
    {
        for (u32 iThread = MainThreadIndex + 1; iThread < (u32)mThreadData.size(); ++iThread)
            mThreadData[iThread]->JoinAndDestroyOnMain();
    }

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

    Module* ConcurrencyModuleCtor(const EngineInitArgMap&)
    {
        return new ConcurrencyModuleImpl();
    }
    ExportInternalModule(Concurrency, ModuleExportInfo(ConcurrencyModuleCtor, true));
}