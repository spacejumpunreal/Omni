#include "Runtime/Engine/EnginePCH.h"
#include "Runtime/Engine/Render/DemoRendererModule.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Concurrency/JobPrimitives.h"
#include "Runtime/Core/System/ModuleExport.h"
#include "Runtime/Core/System/ModuleImplHelpers.h"
#include "Runtime/Core/GfxApi/GfxApiModule.h"
#include "Runtime/Core/Platform/WindowModule.h"
#include "Runtime/Engine/Timing/TimingModule.h"

namespace Omni
{
    /**
    * declarations
    */
    struct DemoRendererModulePrivateImpl
    {
    public:
        void Tick();
        static void _Tick(DemoRendererModulePrivateImpl** impl) { (**impl).Tick(); }
    public:
        SharedPtr<GfxApiSwapChain> SwapChain;
        DispatchWorkItem* TickRegistry;
    };

    /**
    * constants
    */
    static constexpr u32 DemoRendererTickPriority = 100;



    using DemoRendererImpl = PImplCombine<DemoRendererModule, DemoRendererModulePrivateImpl>;

    void DemoRendererModule::Initialize(const EngineInitArgMap& args)
    {
        MemoryModule& mm = MemoryModule::Get();
        mm.Retain();
        WindowModule& wm = WindowModule::Get();
        wm.Retain();
        TimingModule& tm = TimingModule::Get();
        tm.Retain();
        GfxApiModule& gfxApi = GfxApiModule::Get();
        gfxApi.Retain();
        
        DemoRendererImpl& self = *DemoRendererImpl::GetCombinePtr(this);
        //create swapchain
        GfxApiSwapChainDesc descSwapChain;
        descSwapChain.BufferCount = 3;
        descSwapChain.Width = 800;
        descSwapChain.Height = 600;
        descSwapChain.Format = GfxApiFormat::R8G8B8A8_UNORM;
        descSwapChain.WindowHandle = wm.GetMainWindowHandle();
        self.SwapChain = gfxApi.CreateGfxApiObject(descSwapChain);
        self.SwapChain->Present(true);

        
#if 1
        DemoRendererModulePrivateImpl* selfPtr = static_cast<DemoRendererModulePrivateImpl*>(&self);
#if 1
        self.TickRegistry = &DispatchWorkItem::Create<DemoRendererModulePrivateImpl*>(
            DemoRendererModulePrivateImpl::_Tick, 
            &selfPtr, 
            MemoryKind::GfxApi,
            false);
#endif
        //tm.RegisterFrameTick_OnAnyThread(EngineFrameType::Render, DemoRendererTickPriority, *self.TickRegistry, QueueKind::Main);
        //tm.SetFrameRate_OnMainThread(EngineFrameType::Render, 30);
#endif
        Module::Initialize(args);
    }

    void DemoRendererModule::Finalize()
    {
        Module::Finalizing();
        if (GetUserCount() > 0)
            return;

        TimingModule& tm = TimingModule::Get();
        GfxApiModule& gfxApi = GfxApiModule::Get();
        WindowModule& wm = WindowModule::Get();
        MemoryModule& mm = MemoryModule::Get();
        DemoRendererImpl& self = *DemoRendererImpl::GetCombinePtr(this);
        
        self.SwapChain.Clear();
#if 1
        self.TickRegistry->Release(false);
        //tm.UnregisterFrameTick_OnAnyThread(EngineFrameType::Render, DemoRendererTickPriority);
#endif
        Module::Finalize();
        gfxApi.Release();
        tm.Release();
        wm.Release();
        mm.Release();
        
    }

    void DemoRendererModulePrivateImpl::Tick()
    {
    }

    static Module* DemoRendererModuleCtor(const EngineInitArgMap&)
    {
        return InitMemFactory<DemoRendererImpl>::New();
    }

    void DemoRendererModule::Destroy()
    {
        InitMemFactory<DemoRendererImpl>::Delete((DemoRendererImpl*)this);
    }

    EXPORT_INTERNAL_MODULE(DemoRenderer, ModuleExportInfo(DemoRendererModuleCtor, true));
}