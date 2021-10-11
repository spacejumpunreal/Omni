#include "Runtime/Engine/EnginePCH.h"
#include "Runtime/Engine/Timing/TimingModule.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/System/ModuleExport.h"
#include "Runtime/Core/System/ModuleImplHelpers.h"

namespace Omni
{
    struct EngineFrameTickItem
    {
        SimpleCallback Callback;
        u32 Priority;
    };

    struct TimingModulePrivateImpl
    {
    public:
        PMRVector<EngineFrameTickItem>      FrameItems[(u32)EngineFrameType::Count];
        PMRMap<TimePoint, SimpleCallback>   TimerCallbacks;
    public:
    };

    using TimingImpl = PImplCombine<TimingModule, TimingModulePrivateImpl>;

    void TimingModule::Initialize(const EngineInitArgMap& args)
    {
        MemoryModule& mm = MemoryModule::Get();
        mm.Retain();
        Module::Initialize(args);
    }

    void TimingModule::Finalize()
    {
        Module::Finalizing();
        if (GetUserCount() > 0)
            return;

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

    EXPORT_INTERNAL_MODULE(Timing, ModuleExportInfo(TimingModuleCtor, true));

    void TimingModule::AddTimerCallback(TimePoint timePoint, SimpleCallback&& callback)
    {

    }

    void TimingModule::RegisterFrameTick(EngineFrameType frameType, u32 priority)
    {}

    void TimingModule::UnregisterFrameTick(EngineFrameType frameType, u32 priority)
    {}


}