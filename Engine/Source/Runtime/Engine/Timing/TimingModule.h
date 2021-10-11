#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Engine/EngineAPI.h"
#include "Runtime/Core/System/Module.h"
#include <chrono>
#include <functional>

namespace Omni
{
    enum class EngineFrameType : u32
    {
        Gameplay,
        Render,
        Count,
    };

    using SimpleCallback = std::function<void()>;
    using TimePoint = std::chrono::steady_clock::time_point;

    class TimingModule : public Module
    {
    public:
        void Destroy() override;
        void Initialize(const EngineInitArgMap&) override;
        void Finalize() override;

        /**
        *   callback
        */
        void AddTimerCallback(TimePoint timePoint, SimpleCallback&& callback);

        /**
        *   frame
        */
        void RegisterFrameTick(EngineFrameType frameType, u32 priority);
        void UnregisterFrameTick(EngineFrameType frameType, u32 priority);

    };
}