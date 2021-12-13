#include "Runtime/Engine/EnginePCH.h"
#include "Runtime/Engine/Render/DemoRendererModule.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/System/ModuleExport.h"
#include "Runtime/Core/System/ModuleImplHelpers.h"
#include "Runtime/Core/GfxApi/GfxApiModule.h"
#include "Runtime/Core/Platform/WindowModule.h"

namespace Omni
{
    struct DemoRendererModulePrivateImpl
    {
    public:
    public:
        SharedPtr<GfxApiSwapChain> SwapChain;
    };

    using DemoRendererImpl = PImplCombine<DemoRendererModule, DemoRendererModulePrivateImpl>;

    void DemoRendererModule::Initialize(const EngineInitArgMap& args)
    {
        MemoryModule& mm = MemoryModule::Get();
        mm.Retain();
        WindowModule& wm = WindowModule::Get();
        wm.Retain();
        GfxApiModule& gfxApi = GfxApiModule::Get();
        gfxApi.Retain();
        
#if 0
        DemoRendererImpl& self = *DemoRendererImpl::GetCombinePtr(this);
        //create swapchain
        GfxApiSwapChainDesc descSwapChain;
        descSwapChain.BufferCount = 3;
        descSwapChain.Width = 800;
        descSwapChain.Height = 600;
        descSwapChain.Format = GfxApiFormat::R8G8B8A8_UNORM;
        self.SwapChain = gfxApi.CreateGfxApiObject(descSwapChain);
        self.SwapChain->Present();
#endif
        Module::Initialize(args);
    }

    void DemoRendererModule::Finalize()
    {
        Module::Finalizing();
        if (GetUserCount() > 0)
            return;

        Module::Finalize();
        GfxApiModule& gfxApi = GfxApiModule::Get();
        gfxApi.Release();
        WindowModule& wm = WindowModule::Get();
        wm.Release();
        MemoryModule& mm = MemoryModule::Get();
        mm.Release();
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