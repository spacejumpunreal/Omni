#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Engine/EngineAPI.h"
#include "Runtime/Base/Misc/TimeTypes.h"
#include "Runtime/Core/Concurrency/ConcurrentDefs.h"
#include "Runtime/Core/System/Module.h"

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

    class FrameContext
    {
    public:
        u64 GetFrameIndex();
        TimePoint GetTime();
    private:
        FrameContext();
    };

    /*
    * mostly polled by MainThread, callback can be manipulated from any thread
    * won't use a dedicated thread for timer callback because it's not only a little complicated but
    * also make timing difficult to analyze
    */
    class ENGINE_API TimingModule : public Module
    {
    public:
        void Initialize(const EngineInitArgMap&) override;
        void StopThreads() override;
        void Finalize() override;
        void Destroy() override;
        static TimingModule& Get();

        /**
        *   callback
        */
        void AddTimerCallback_OnAnyThread(TimePoint timePoint, DispatchWorkItem& workItem, QueueKind queue);

        /**
        *   frame
        */
        void SetTickInterval_OnMainThread(Duration duration);
        void SetFrameRate_OnMainThread(EngineFrameType frameType, u32 ticksPerFrame);
        void RegisterFrameTick_OnAnyThread(EngineFrameType frameType, u32 priority, DispatchWorkItem& callback, QueueKind queue);
        void UnregisterFrameTick_OnAnyThread(EngineFrameType frameType, u32 priority);

    };
}