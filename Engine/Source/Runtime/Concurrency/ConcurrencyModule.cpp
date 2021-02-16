#include "Runtime/Concurrency/ConcurrencyModule.h"
#include "Runtime/Concurrency/ConcurrentContainers.h"
#include "Runtime/Concurrency/ConcurrentDefs.h"
#include "Runtime/Concurrency/SpinLock.h"
#include "Runtime/Concurrency/ThreadUtils.h"
#include "Runtime/Memory/MemoryModule.h"
#include "Runtime/Misc/Padding.h"
#include "Runtime/Misc/PImplUtils.h"
#include "Runtime/System/ModuleExport.h"
#include "Runtime/System/Module.h"

#include <Windows.h>
namespace Omni
{
    //forard decalrations
    struct ConcurrencyModulePrivateImpl
    {
    public:
        DispatchQueue       mSerialQueues[(u32)QueueKind::Max];
        ConcurrentQueue     mSharedQueue;
        ThreadData**        mThreadData;
        u32                 mThreadCount;
    public:
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
        u32 nThreads = self->mThreadCount = (u32)std::thread::hardware_concurrency();
        //nThreads = 1;
        self->mThreadCount = nThreads;
        self->mThreadData = AllocForType<ThreadData*, MemoryKind::SystemInit>(nThreads);
        for (u32 iThread = 0; iThread < nThreads; ++iThread)
        {
            self->mThreadData[iThread] = &ThreadData::Create();
        }
        self->mThreadData[0]->InitAsMainOnMain();
        for (u32 iThread = 1; iThread < nThreads; ++iThread)
        {
            self->mThreadData[iThread]->LauchAsWorkerOnMain();
        }
    }
    void ConcurrencyModule::Finalize()
    {
        Module::Finalizing();
        if (GetUserCount() > 0)
            return;

        ConcurrencyModuleImpl* self = ConcurrencyModuleImpl::GetCombinePtr(this);
        for (u32 iThread = 0; iThread < self->mThreadCount; ++iThread)
            self->mThreadData[iThread]->AskQuitOnMain();
        for (u32 iThread = 0; iThread < self->mThreadCount; ++iThread)
            self->mThreadData[iThread]->JoinAndDestroyOnMain();
        FreeForType<ThreadData*, MemoryKind::SystemInit>(self->mThreadData, self->mThreadCount);
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

    void ConcurrentWorkerThreadFunc(ThreadData* self)
    {
        ConcurrencyModuleImpl* cm = (ConcurrencyModuleImpl*)gConcurrencyModule;
        while (!self->IsAskedToQuit())
        {
            DispatchWorkItem* item = cm->mSharedQueue.Dequeue<DispatchWorkItem>();
            item->Perform();
            item->Destroy();
        }
    }


    Module* ConcurrencyModuleCtor(const EngineInitArgMap&)
    {
        return new ConcurrencyModuleImpl();
    }
    ExportInternalModule(Concurrency, ModuleExportInfo(ConcurrencyModuleCtor, true));
}