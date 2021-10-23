#include "Runtime/Engine/EnginePCH.h"
#include "Runtime/Engine/Timing/TimingModule.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Base/MultiThread/SpinLock.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Base/Memory/MemoryArena.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Concurrency/ConcurrencyModule.h"
#include "Runtime/Core/System/ModuleExport.h"
#include "Runtime/Core/System/ModuleImplHelpers.h"

namespace Omni
{
    //forward decls
    struct TimingModulePrivateImpl;

    struct EngineFrameTickItem
    {
        DispatchWorkItem*   WorkItem;
        QueueKind           Queue;
        u32                 Priority;
    };

    struct TimerCallbackItem
    {
        DispatchWorkItem*   WorkItem;
        QueueKind           Queue;
    };

    struct TriggerFrameTask
    {
        TimingModulePrivateImpl*    Self;
        EngineFrameType             Type;

        static void DoTask(TriggerFrameTask* data);
    };

    enum class ClockThreadState : u32
    {
        NotRunning,
        Running,
        MarkedToStop,
        Stopped,
    };


    struct FrameContextPrivate
    {
    public:
        u64         mFrameIndex;
        TimePoint   mTimePoint;
    };
    using FrameContextImpl = PImplCombine<FrameContext, FrameContextPrivate>;

    struct TimingModulePrivateImpl
    {
        static constexpr u32 QueueCount = (u32)EngineFrameType::Count;
    public:
        std::mutex                              mLock;
        std::condition_variable                 mCV;
        PMRVector<EngineFrameTickItem>          mFrameTickQueue[QueueCount];
        Duration                                mFrameDurations[QueueCount];
        PMRMap<TimePoint, TimerCallbackItem>    mTimerCallbacks;

        std::thread                             mClockThread;
        std::atomic<ClockThreadState>           mThreadState = ClockThreadState::NotRunning;
    public:
        TimingModulePrivateImpl();
        void TimingModuleLoop();
        void OnFrameBegin(EngineFrameType frameType);
        void EnqueueFrameTickCallback(EngineFrameType frameType, Duration& duration);
    };

    using TimingImpl = PImplCombine<TimingModule, TimingModulePrivateImpl>;


    /**
      * TimingModule
      */
    u64 FrameContext::GetFrameIndex()
    {
        FrameContextImpl* self = FrameContextImpl::GetCombinePtr(this);
        return self->mFrameIndex;
    }

    TimePoint FrameContext::GetTime()
    {
        FrameContextImpl* self = FrameContextImpl::GetCombinePtr(this);
        return self->mTimePoint;
    }

    /**
      * TimingModule
      */
    void TimingModule::Initialize(const EngineInitArgMap& args)
    {
        MemoryModule& mm = MemoryModule::Get();
        mm.Retain();

        TimingImpl* self = TimingImpl::GetCombinePtr(this);
        for (u32 iFrameType = 0; iFrameType < TimingModulePrivateImpl::QueueCount; ++iFrameType)
        {
            self->EnqueueFrameTickCallback((EngineFrameType)iFrameType, self->mFrameDurations[iFrameType]);
        }

        self->mClockThread = std::thread([self]()
        {
            self->TimingModuleLoop();
        });

        Module::Initialize(args);
    }

    void TimingModule::Finalize()
    {
        Module::Finalizing();
        if (GetUserCount() > 0)
            return;

        TimingImpl* self = TimingImpl::GetCombinePtr(this);
        self->mThreadState.store(ClockThreadState::MarkedToStop, std::memory_order_release);
        while (self->mThreadState.load(std::memory_order_acquire) != ClockThreadState::Stopped)
        {
            std::this_thread::yield();
        }

        MemoryModule& mm = MemoryModule::Get();
        Module::Finalize();
        mm.Release();
    }

    static Module* TimingModuleCtor(const EngineInitArgMap&)
    {
        return InitMemFactory<TimingImpl>::New();
    }

    void TimingModule::Destroy()
    {
        InitMemFactory<TimingImpl>::Delete((TimingImpl*)this);
    }

    void TimingModule::AddTimerCallback(TimePoint timePoint, DispatchWorkItem& callback, QueueKind queue)
    {
        TimingImpl* self = TimingImpl::GetCombinePtr(this);
        self->mLock.lock();

        auto prevBegin = self->mTimerCallbacks.begin();
        auto ret = self->mTimerCallbacks.emplace(std::make_pair(timePoint, TimerCallbackItem{ .WorkItem = &callback, .Queue = queue }));
        if (prevBegin != ret.first)
        {//inserted a new head
            self->mCV.notify_one();
        }

        self->mLock.unlock();
    }

    void TimingModule::SetFrameRate(EngineFrameType frameType, Duration duration)
    {
        (void)frameType;
        (void)duration;
    }

    void TimingModule::RegisterFrameTick(EngineFrameType frameType, u32 priority, DispatchWorkItem& callback, QueueKind queue)
    {
        TimingImpl* self = TimingImpl::GetCombinePtr(this);
        self->mLock.lock();

        PMRVector<EngineFrameTickItem>& frameQ = self->mFrameTickQueue[(u32)frameType];
        auto it = frameQ.begin();
        for (; it != frameQ.end(); ++it)
        {
            if (it->Priority > priority)
                break;
            else if (it->Priority == priority)
            {
                CheckAlways(false, "item with same priority already registered");
            }
        }
        frameQ.insert(it, EngineFrameTickItem{
            .WorkItem = &callback,
            .Queue = queue,
            .Priority = priority,
            });

        self->mLock.unlock();
    }

    void TimingModule::UnregisterFrameTick(EngineFrameType frameType, u32 priority)
    {
        TimingImpl* self = TimingImpl::GetCombinePtr(this);
        self->mLock.lock();

        PMRVector<EngineFrameTickItem>& frameQ = self->mFrameTickQueue[(u32)frameType];
        auto it = frameQ.begin();
        for (; it != frameQ.end(); ++it)
        {
            if (it->Priority == priority)
            {
                frameQ.erase(it);
            }
        }
        self->mLock.unlock();
    }

    EXPORT_INTERNAL_MODULE(Timing, ModuleExportInfo(TimingModuleCtor, true));

    /**
      * TimingModulePrivateImpl
      */
    TimingModulePrivateImpl::TimingModulePrivateImpl()
        : mFrameTickQueue { 
            PMRVector<EngineFrameTickItem>(MemoryModule::Get().GetPMRAllocator(MemoryKind::SystemInit)),
            PMRVector<EngineFrameTickItem>(MemoryModule::Get().GetPMRAllocator(MemoryKind::SystemInit)),
        }
        , mTimerCallbacks(MemoryModule::Get().GetPMRAllocator(MemoryKind::SystemInit))
    {
        for (u32 i = 0; i < QueueCount; ++i)
        {
            mFrameDurations[i] = std::chrono::milliseconds(33);
        }
    }

    void TriggerFrameTask::DoTask(TriggerFrameTask* data)
    {
        data->Self->OnFrameBegin(data->Type);
    }

    template<typename TItem>
    static void HandleATask(TItem& item)
    {
        (void)item;
#if false
        if (item.Queue == QueueKind::Immediate)
        {
            item.WorkItem->Perform();
            item.WorkItem->Destroy();
        }
        else if (item.Queue == QueueKind::Shared)
            ConcurrencyModule::Get().Async(*item.WorkItem);
        else
        {
            ConcurrencyModule::Get().GetQueue(item.Queue).Enqueue(item.WorkItem, item.WorkItem);
            CheckAlways(item.WorkItem->Next == nullptr);
        }
#endif
    }

    void TimingModulePrivateImpl::TimingModuleLoop()
    {
        mThreadState.store(ClockThreadState::Running, std::memory_order_release);
        std::unique_lock lk(mLock);
        while (mThreadState.load(std::memory_order_relaxed) != ClockThreadState::MarkedToStop)
        {
            TimePoint now = TClock::now();
            Duration toWait;
            while (true)
            {
                auto& ptAndCallback = *mTimerCallbacks.begin();
                toWait = ptAndCallback.first - now;
                if (toWait.count() >= 0)
                {
                    break;
                }
                //else, callback is due
                HandleATask(ptAndCallback.second);
            }
            if (mTimerCallbacks.size() > 0)
            {
                mCV.wait_for(lk, toWait);
            }
            else
            {
                mCV.wait(lk);
            }
        }
        //won't call remainning callbacks, since the protocol is to call them when time is due
        for (auto it = mTimerCallbacks.begin(); it != mTimerCallbacks.end(); ++it)
        {
            it->second.WorkItem->Destroy();
        }

        mThreadState.store(ClockThreadState::Stopped, std::memory_order_acq_rel);
    }

    void TimingModulePrivateImpl::OnFrameBegin(EngineFrameType frameType)
    {//when this is executed, mLock is already held
        CheckDebug(mLock.try_lock() == false);
        ScratchStack& stk = MemoryModule::Get().GetThreadScratchStack();
        stk.Push();

        PMRVector<EngineFrameTickItem>& queue = mFrameTickQueue[(u32)frameType];
        size_t nJobs = queue.size();
        auto items = (EngineFrameTickItem*)stk.AllocateAndInitWith((u32)nJobs, queue.data());

        for (size_t i = 0; i < nJobs; ++i)
        {
            HandleATask(items[i]);
        }
        stk.Pop();
    }

    void TimingModulePrivateImpl::EnqueueFrameTickCallback(EngineFrameType frameType, Duration& duration)
    {
        (void)frameType;
        (void)duration;
#if false
        TimingImpl* self = TimingImpl::GetCombinePtr(this);
        TriggerFrameTask td;
        td.Self = this;
        td.Type = frameType;
        DispatchWorkItem& workItem = DispatchWorkItem::Create<TriggerFrameTask>(
            TriggerFrameTask::DoTask, 
            &td, MemoryKind::CacheLine);

        TimePoint now = TClock::now();
        self->AddTimerCallback(now + duration, workItem, QueueKind::Immediate);
#endif
    }
}