#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/Concurrency/ConcurrencyModule.h"
#include "Runtime/Core/Concurrency/ThreadUtils.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/MultiThread/IThreadLocal.h"
#include "Runtime/Base/MultiThread/LockfreeContainer.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Platform/WindowModule.h"
#include <atomic>
#include <thread>


namespace Omni
{
    //forward decls
    struct ThreadDataImpl;
    extern void ConcurrentWorkerThreadFunc(ThreadData*);


    //global data
    static std::atomic<u32>                         gThreadCount;
    static std::thread::id                          gMainThreadId;
    OMNI_DECLARE_THREAD_LOCAL(ThreadDataImpl*,      gThreadData);


    //declaration
    struct ThreadDataImpl : public ThreadData
    {
        //using WorkerThreadSignature = void(ThreadData* td);
    public:
        std::thread         mThread;
        u32                 mThreadId;
        bool                mQuitAsked;
        std::atomic<bool>   mFinalized;
    public:
        ThreadDataImpl(ThreadId tid); //only called on main thread, worker thread not created actually
    };


    bool IsOnMainThread()
    {
        return gMainThreadId == std::this_thread::get_id();
    }

    void RegisterMainThread()
    {
        CheckAlways(gMainThreadId == std::thread::id{});
        gMainThreadId = std::this_thread::get_id();
    }

    void UnregisterMainThread()
    {
        CheckAlways(IsOnMainThread());
        gMainThreadId = std::thread::id{};
    }

    ThreadData& ThreadData::Create(ThreadId tid)
    {
        return *OMNI_NEW(MemoryKind::SystemInit) ThreadDataImpl(tid);
    }

    void ThreadData::InitAsMainOnMain()
    {
        auto& self = *static_cast<ThreadDataImpl*>(this);
        CheckAlways(IsOnMainThread());
        self.InitializeOnThread();
    }

    void ThreadData::RunAndFinalizeAsMain(SystemInitializedCallback initCb, SystemWillQuitCallback quitCb)
    {
        auto& self = *static_cast<ThreadDataImpl*>(this);
        CheckAlways(IsOnMainThread());
        
        ConcurrencyModule::Get().EnqueueWork(
            DispatchWorkItem::Create(initCb, MemoryKind::CacheLine),
            QueueKind::Main);
        auto queue = ConcurrencyModule::Get().GetQueue(QueueKind::Main);

        while (!self.IsAskedToQuit())
        {
            auto item = queue->Dequeue<DispatchWorkItem>();
            item->Perform();
            item->Destroy();
        }
        if (quitCb != nullptr)
            quitCb();
        System::GetSystem().StopThreads();
        System::GetSystem().Finalize();
    }

    void ThreadData::LauchAsWorkerOnMain(const TThreadBody& body)
    {
        auto& self = *static_cast<ThreadDataImpl*>(this);
        self.mThread = std::thread([body, &self] {
            self.InitializeOnThread();
            body();
            self.FinalizeOnThread();
        });
    }

    void ThreadData::JoinAndDestroyOnMain()
    {
        CheckAlways(IsOnMainThread());
        auto self = static_cast<ThreadDataImpl*>(this);
        if (self->GetThreadId() == MainThreadId)
        {
            self->FinalizeOnThread(); //main thread need to do something during some finalization, call its FinalizeOnThread() here
        }
        else
        {
            self->mThread.join(); //this is not main thread, wait for it
        }
        OMNI_DELETE(self, MemoryKind::SystemInit); //memory for this is allocated on Main
    }

    void ThreadData::CheckFinalizedAndDestroyOnMain()
    {
        CheckAlways(IsOnMainThread());
        auto self = static_cast<ThreadDataImpl*>(this);
        CheckAlways(self->IsFinalized());
        OMNI_DELETE(self, MemoryKind::SystemInit);
    }

    ThreadData& ThreadData::GetThisThreadData()
    {
        return *gThreadData.GetRaw();
    }

    bool ThreadData::IsAskedToQuit()
    {
        auto& self = *static_cast<ThreadDataImpl*>(this);
        return self.mQuitAsked;
    }

    void ThreadData::MarkQuitWork()
    {
        gThreadData.GetRaw()->mQuitAsked = true;
    }

    bool ThreadData::IsSelfThread()
    {
        return gThreadData.GetRaw() == this;
    }

    bool ThreadData::IsFinalized()
    {
        auto& self = *static_cast<ThreadDataImpl*>(this);
        return self.mFinalized;
    }

    ThreadId ThreadData::GetThreadId()
    {
        auto& self = *static_cast<ThreadDataImpl*>(this);
        return self.mThreadId;
    }

    ThreadDataImpl::ThreadDataImpl(ThreadId tid)
        : mThreadId(tid)
        , mQuitAsked(false)
        , mFinalized(false)
    {
        CheckAlways(IsOnMainThread());
    }

    void ThreadData::InitializeOnThread()
    {
        auto& self = *static_cast<ThreadDataImpl*>(this);
        gThreadCount.fetch_add(1);
        gThreadData.GetRaw() = &self;
        MemoryModule::ThreadInitialize();
        
    }

    void ThreadData::FinalizeOnThread()
    {
        auto& self = *static_cast<ThreadDataImpl*>(this);
        MemoryModule::ThreadFinalize();
        gThreadCount.fetch_sub(1);
        gThreadData.GetRaw() = nullptr;
        self.mFinalized = true;
    }
}

