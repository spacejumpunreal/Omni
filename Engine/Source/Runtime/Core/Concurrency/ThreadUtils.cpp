#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/Concurrency/ThreadUtils.h"
#include "Runtime/Base/MultiThread/IThreadLocal.h"
#include "Runtime/Base/MultiThread/LockfreeContainer.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Platform/WindowModule.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include <atomic>
#include <thread>


namespace Omni
{
    struct ThreadDataImpl;
    extern void ConcurrentWorkerThreadFunc(ThreadData*);

    static std::atomic<ThreadIndex>                 gThreadCount;
    static std::thread::id                          gMainThreadId;
    OMNI_DECLARE_THREAD_LOCAL(ThreadDataImpl*,      gThreadData);

    struct ThreadDataImpl : public ThreadData
    {
        //using WorkerThreadSignature = void(ThreadData* td);
    public:
        std::thread         mThread;
        ThreadIndex         mThreadId;
        bool                mQuitAsked;
    public:
        ThreadDataImpl(); //only called on main thread, worker thread not created actually
        void InitializeOnThread(); //only called on self thread
        void FinalizeOnThread(); //only called on self thread
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
    ThreadData& ThreadData::Create()
    {
        return *OMNI_NEW(MemoryKind::SystemInit) ThreadDataImpl();
    }
    void ThreadData::InitAsMainOnMain()
    {
        auto& self = *static_cast<ThreadDataImpl*>(this);
        CheckAlways(IsOnMainThread());
        self.InitializeOnThread();
    }
    void ThreadData::RunAndFinalizeOnMain(SystemInitializedCallback onSystemInitialized)
    {
        auto& self = *static_cast<ThreadDataImpl*>(this);
        CheckAlways(IsOnMainThread());
        onSystemInitialized();
        WindowModule* win = WindowModule::GetPtr();
        if (win)
            win->RunUILoop();
        ConcurrentWorkerThreadFunc(&self);
        System::GetSystem().Finalize();
    }
    void ThreadData::LauchAsWorkerOnMain()
    {
        auto& self = *static_cast<ThreadDataImpl*>(this);
        self.mThread = std::thread([&self] { 
            self.InitializeOnThread();
            ConcurrentWorkerThreadFunc(&self);
            self.FinalizeOnThread();
        });
    }
    void ThreadData::JoinAndDestroyOnMain()
    {
        CheckAlways(IsOnMainThread());
        auto self = static_cast<ThreadDataImpl*>(this);
        if (self->GetThreadIndex() == 0) 
            self->FinalizeOnThread(); //main thread is not ThreadFinalized yet(may still need stuff during some finalization), do it here
        else
            self->mThread.join(); //this is not main thread, wait for it
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
    bool ThreadData::IsOnSelfThread()
    {
        return gThreadData.GetRaw() == this;
    }
    ThreadIndex ThreadData::GetThreadIndex()
    {
        auto& self = *static_cast<ThreadDataImpl*>(this);
        return self.mThreadId;
    }
    ThreadDataImpl::ThreadDataImpl()
        : mThreadId(InvalidThreadIndex)
        , mQuitAsked(false)
    {
        CheckAlways(IsOnMainThread());
    }
    void ThreadDataImpl::InitializeOnThread()
    {
        mThreadId = gThreadCount.fetch_add(1);
        gThreadData.GetRaw() = this;
        MemoryModule::ThreadInitialize();
        
    }
    void ThreadDataImpl::FinalizeOnThread()
    {
        MemoryModule::ThreadFinalize();
        gThreadCount.fetch_sub(1);
        gThreadData.GetRaw() = nullptr;
    }
}

