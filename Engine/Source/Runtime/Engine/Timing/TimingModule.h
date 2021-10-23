#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Engine/EngineAPI.h"
#include "Runtime/Core/Concurrency/ConcurrentDefs.h"
#include "Runtime/Core/System/Module.h"

#include <chrono>

namespace Omni
{
    //forward declarations
    class DispatchWorkItem;
    enum class QueueKind : u32;


    enum class EngineFrameType : u32
    {
        Gameplay,
        Render,
        Count,
    };

    using TClock = std::chrono::steady_clock;
    using TimePoint = TClock::time_point;
    using Duration = TClock::duration;

    class FrameContext
    {
    public:
        u64 GetFrameIndex();
        TimePoint GetTime();
    private:
        FrameContext();
    };


    class TimingModule : public Module
    {
    public:
        void Destroy() override;
        void Initialize(const EngineInitArgMap&) override;
        void Finalize() override;

        /**
        *   callback
        */
        void AddTimerCallback(TimePoint timePoint, DispatchWorkItem& workItem, QueueKind queue);

        /**
        *   frame
        */
        void SetFrameRate(EngineFrameType frameType, Duration duration);
        void RegisterFrameTick(EngineFrameType frameType, u32 priority, DispatchWorkItem& callback, QueueKind queue);
        void UnregisterFrameTick(EngineFrameType frameType, u32 priority);

    };
}