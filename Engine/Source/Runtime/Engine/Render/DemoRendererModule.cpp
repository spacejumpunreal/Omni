#include "Runtime/Engine/EnginePCH.h"
#include "Runtime/Engine/Render/DemoRendererModule.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Concurrency/JobPrimitives.h"
#include "Runtime/Core/System/ModuleExport.h"
#include "Runtime/Core/System/ModuleImplHelpers.h"
#include "Runtime/Core/GfxApi/GfxApiModule.h"
#include "Runtime/Core/GfxApi/GfxApiRenderPass.h"
#include "Runtime/Core/Platform/WindowModule.h"
#include "Runtime/Engine/Timing/TimingModule.h"


#define DEMO_MODULE 1

namespace Omni
{
    /**
    * Constants
    */
    constexpr u32 BackbufferCount = 3;

    /**
    * Declarations
    */

    struct DemoRendererModulePrivateImpl : public ICallback
    {
    public:
        DemoRendererModulePrivateImpl();
        void Tick();
        void operator()() override { Tick(); }
    public:
        DispatchWorkItem*               TickRegistry = nullptr;
        GfxApiSwapChainRef              SwapChain = {};
        GfxApiTextureRef                Backbuffers[BackbufferCount];
        u32                             ClientAreaSizeX = 0;
        u32                             ClientAreaSizeY = 0;
        u32                             FrameIndex = 0;
    };

    using DemoRendererImpl = PImplCombine<DemoRendererModule, DemoRendererModulePrivateImpl>;

    /**
    * constants
    */
    static constexpr u32 DemoRendererTickPriority = 100;

    /**
    * definitions
    */

    DemoRendererModulePrivateImpl::DemoRendererModulePrivateImpl()
    {
    }

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

#if DEMO_MODULE
        DemoRendererImpl& self = *DemoRendererImpl::GetCombinePtr(this);
        //create swapchain
        wm.GetClientAreaSize(self.ClientAreaSizeX, self.ClientAreaSizeY);
        GfxApiSwapChainDesc descSwapChain;
        descSwapChain.BufferCount = BackbufferCount;
        descSwapChain.Width = self.ClientAreaSizeX;
        descSwapChain.Height = self.ClientAreaSizeY;
        descSwapChain.Format = GfxApiFormat::R8G8B8A8_UNORM;
        descSwapChain.WindowHandle = wm.GetMainWindowHandle();
        self.SwapChain = gfxApi.CreateSwapChain(descSwapChain);
        gfxApi.GetBackbufferTextures(self.SwapChain, self.Backbuffers, BackbufferCount);
#endif
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
        

#if DEMO_MODULE
        DemoRendererImpl& self = *DemoRendererImpl::GetCombinePtr(this);
        gfxApi.DestroySwapChain(self.SwapChain);
        self.SwapChain = nullptr;
        for (u32 iBuffer = 0; iBuffer < BackbufferCount; ++iBuffer)
        {
            self.Backbuffers[iBuffer] = nullptr;
        }
        tm.UnregisterFrameTick_OnAnyThread(EngineFrameType::Render, DemoRendererTickPriority);
#endif
        Module::Finalize();
        gfxApi.Release();
        tm.Release();
        wm.Release();
        mm.Release();
        
    }

    void DemoRendererModulePrivateImpl::Tick()
    {
#if DEMO_MODULE
        GfxApiModule& gfxApi = GfxApiModule::Get();
        
        DemoRendererImpl& self = *DemoRendererImpl::GetCombinePtr(this);
        gfxApi.CheckGpuEvents(AllQueueMask);

        {
            u32 cwidth, cheight;
            WindowModule& wm = WindowModule::Get();
            wm.GetClientAreaSize(cwidth, cheight);
            if (cwidth != self.ClientAreaSizeX || cheight != self.ClientAreaSizeY)
            {
                self.ClientAreaSizeX = cwidth;
                self.ClientAreaSizeY = cheight;
                GfxApiSwapChainDesc newDesc = self.SwapChain->GetDesc();
                newDesc.Width = cwidth;
                newDesc.Height = cheight;
                gfxApi.UpdateSwapChain(self.SwapChain, newDesc);
                gfxApi.GetBackbufferTextures(self.SwapChain, self.Backbuffers, BackbufferCount);
            }
        }

        u32 currentBuffer = gfxApi.GetCurrentBackbufferIndex(self.SwapChain);
        GfxApiRenderPass* renderPass = new GfxApiRenderPass(0);
        renderPass->RenderTargets[0].Texture = self.Backbuffers[currentBuffer];
        renderPass->RenderTargets[0].ClearValue = Vector4(1, 0, 0, 0);
        renderPass->RenderTargets[0].Action = GfxApiLoadStoreActions::Clear | GfxApiLoadStoreActions::Store;

        gfxApi.DrawRenderPass(renderPass, nullptr);
        gfxApi.Present(self.SwapChain, true, nullptr);
        gfxApi.ScheduleGpuEvent(GfxApiQueueType::GraphicsQueue, nullptr);
#endif
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

#undef DEMO_MODULE
