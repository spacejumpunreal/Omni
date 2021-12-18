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
#include "Runtime/Base/Misc/SharedObject.h"

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
        PMRVector<EngineFrameTickItem>          mFrameTickQueue[QueueCount];
        PMRMap<TimePoint, TimerCallbackItem>    mTimerCallbacks;
        u32                                     mTicksPerFrame[QueueCount];
        u32                                     mFrameTicks[QueueCount];
        TimePoint                               mLastTickTime;
        Duration                                mTickDuration;
        bool                                    mStopTick;
    public:
        TimingModulePrivateImpl();
        void DoTick();
    };

    using TimingImpl = PImplCombine<TimingModule, TimingModulePrivateImpl>;


    //globals
    TimingModule* gTimingModule;


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

    struct TickFrameJob
    {
        TimingModulePrivateImpl* TimerModule;

        static void Run(TickFrameJob* self)
        {
            self->TimerModule->DoTick();
        }
    };

    void TimingModule::Initialize(const EngineInitArgMap& args)
    {
        MemoryModule& mm = MemoryModule::Get();
        mm.Retain();
        ConcurrencyModule& cm = ConcurrencyModule::Get();
        cm.Retain();

        CheckAlways(gTimingModule == nullptr);
        gTimingModule = this;

        TimingImpl* self = TimingImpl::GetCombinePtr(this);
        self->mLastTickTime = TClock::now();

        cm.EnqueueWork(DispatchWorkItem::CreateWithFunctor(TickFrameJob{ self }, MemoryKind::CacheLine, true), QueueKind::Main);

        Module::Initialize(args);
    }

    void TimingModule::StopThreads()
    {
        TimingImpl* self = TimingImpl::GetCombinePtr(this);
        self->mStopTick = true;
    }

    void TimingModule::Finalize()
    {
        Module::Finalizing();
        if (GetUserCount() > 0)
            return;

        MemoryModule& mm = MemoryModule::Get();
        ConcurrencyModule& cm = ConcurrencyModule::Get();
        mm.Release();
        cm.Release();
        Module::Finalize();
    }

    static Module* TimingModuleCtor(const EngineInitArgMap&)
    {
        return InitMemFactory<TimingImpl>::New();
    }

    void TimingModule::Destroy()
    {
        InitMemFactory<TimingImpl>::Delete((TimingImpl*)this);
    }

    TimingModule& TimingModule::Get()
    {
        return *gTimingModule;
    }

    void TimingModule::AddTimerCallback_OnAnyThread(TimePoint timePoint, DispatchWorkItem& callback, QueueKind queue)
    {
        //TimingImpl* self = TimingImpl::GetCombinePtr(this);
        (void)timePoint;
        (void)callback;
        (void)queue;
    }

    void TimingModule::SetTickInterval_OnMainThread(Duration duration)
    {
        CheckAlways(IsOnMainThread());
        TimingImpl* self = TimingImpl::GetCombinePtr(this);
        self->mTickDuration = duration;
    }

    void TimingModule::SetFrameRate_OnMainThread(EngineFrameType frameType, u32 ticksPerFrame)
    {
        CheckAlways(IsOnMainThread());
        TimingImpl* self = TimingImpl::GetCombinePtr(this);
        self->mTicksPerFrame[(u32)frameType] = ticksPerFrame;
    }

    void TimingModule::RegisterFrameTick_OnAnyThread(EngineFrameType frameType, u32 priority, DispatchWorkItem& callback, QueueKind queue)
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

    void TimingModule::UnregisterFrameTick_OnAnyThread(EngineFrameType frameType, u32 priority)
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
        , mTickDuration(std::chrono::milliseconds(33))
        , mStopTick(false)
    {
        for (u32 i = 0; i < QueueCount; ++i)
        {
            mTicksPerFrame[i] = 1;
            mFrameTicks[i] = 0;
        }
    }

    void TimingModulePrivateImpl::DoTick()
    {
        if (mStopTick)
            return;

        ConcurrencyModule& cm = ConcurrencyModule::Get();
        
        TimePoint now = TClock::now();
        TimePoint waitUntil = mLastTickTime + mTickDuration;
        if (now < waitUntil)
        {
            cm.PollQueueUntil(QueueKind::Shared, &mLastTickTime);
            now = TClock::now();
        }
        //timer callbacks
        {
            PMRVector<TimerCallbackItem> expiredItems(16, MemoryModule::Get().GetPMRAllocator(MemoryKind::CacheLine));
            expiredItems.clear();
            mLock.lock();
            for (auto it = mTimerCallbacks.begin(); it != mTimerCallbacks.end(); ++it)
            {
                if (it->first > now)
                    break;
                expiredItems.emplace_back(std::move(it->second));
            }
            mLock.unlock();
            for (auto& item : expiredItems)
            {
                cm.EnqueueWork(*item.WorkItem, item.Queue);
            }
        }
        //frame ticks
        {
            PMRVector<TimerCallbackItem> todoItems(16, MemoryModule::Get().GetPMRAllocator(MemoryKind::CacheLine));
            todoItems.clear();
            mLock.lock();
            for (u32 iQueue = 0; iQueue < QueueCount; ++iQueue)
            {
                if (mTicksPerFrame[iQueue] <= ++mFrameTicks[iQueue])
                {
                    mFrameTicks[iQueue] = 0;
                    auto& queue = mFrameTickQueue[iQueue];
                    for (EngineFrameTickItem& item : queue)
                    {
                        todoItems.push_back(TimerCallbackItem{
                                .WorkItem = item.WorkItem,
                                .Queue = item.Queue,
                            });
                    }
                }
            }
            mLock.unlock();
            for (auto& item : todoItems)
            {
                cm.EnqueueWork(*item.WorkItem, item.Queue);
            }
        }
        cm.EnqueueWork(DispatchWorkItem::CreateWithFunctor(TickFrameJob{ this }, MemoryKind::CacheLine, true), QueueKind::Main);
        mLastTickTime = now;
    }
}