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

    struct DemoRendererModulePrivateImpl : public ICallback
    {
    public:
        void Tick();
        void operator()() override { Tick(); }
    public:
        DispatchWorkItem*               TickRegistry;
        SharedPtr<GfxApiSwapChain>      SwapChain;
    };

    using DemoRendererImpl = PImplCombine<DemoRendererModule, DemoRendererModulePrivateImpl>;

    /**
    * constants
    */
    static constexpr u32 DemoRendererTickPriority = 100;

    /**
    * definitions
    */

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
        
        u32 w, h;
        wm.GetBackbufferSize(w, h);

        DemoRendererImpl& self = *DemoRendererImpl::GetCombinePtr(this);
        //create swapchain
        GfxApiSwapChainDesc descSwapChain;
        descSwapChain.BufferCount = 3;
        descSwapChain.Width = w;
        descSwapChain.Height = h;
        descSwapChain.Format = GfxApiFormat::R8G8B8A8_UNORM;
        descSwapChain.WindowHandle = wm.GetMainWindowHandle();
        self.SwapChain = gfxApi.CreateGfxApiObject(descSwapChain);
        self.SwapChain->Present(true);
        tm.RegisterFrameTick_OnAnyThread(EngineFrameType::Render, DemoRendererTickPriority, DemoRendererImpl::GetData(this), QueueKind::Main);
        tm.SetFrameRate_OnMainThread(EngineFrameType::Render, 30);
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
        tm.UnregisterFrameTick_OnAnyThread(EngineFrameType::Render, DemoRendererTickPriority);
        Module::Finalize();
        gfxApi.Release();
        tm.Release();
        wm.Release();
        mm.Release();
        
    }

    void DemoRendererModulePrivateImpl::Tick()
    {
        DemoRendererImpl& self = *DemoRendererImpl::GetCombinePtr(this);
        GfxApiModule& gfxApiM = GfxApiModule::Get();

        GfxApiRenderPassDesc passDesc;
        passDesc.Color[0].Texture = self.SwapChain->GetCurrentBackbuffer();
        GfxApiRenderPass* renderPass = gfxApiM.BeginRenderPass(passDesc);
        
        GfxApiCommandContextDesc cmdCtxDesc;
        cmdCtxDesc.RenderPass = renderPass;
        cmdCtxDesc.Type = GfxApiContextType::Render;
        GfxApiCommandContext* cmdCtx = gfxApiM.BeginContext(cmdCtxDesc);

        gfxApiM.EndContext(cmdCtx);

        self.SwapChain->Present(true);
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