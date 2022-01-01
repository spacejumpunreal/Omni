#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Platform/WindowModule.h"
#include "Runtime/Core/System/ModuleExport.h"
#include "Runtime/Core/System/ModuleImplHelpers.h"
#include "Runtime/Core/GfxApi/GfxApiModule.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12SwapChain.h"

#include <d3d12.h>
#include <dxgidebug.h>

#define DEBUG_DX_OBJECT_LEAK_ON_QUIT 1


EXTERN_C const GUID DECLSPEC_SELECTANY DXGI_DEBUG_ALL =  { 0xe48ae283, 0xda80, 0x490b, 0x87, 0xe6, 0x43, 0xe9, 0xa9, 0xcf, 0xda, 0x8  };


namespace Omni
{
    //forward decls
    // 
    //declarations
    class DX12Module final : public GfxApiModule
    {
    public:
        DX12Module(const EngineInitArgMap& args);
        void Destroy() override;
        void Initialize(const EngineInitArgMap&) override;
        void StopThreads() override;
        void Finalize() override;

#define GfxApiMethod(Definition) Definition override;
#include "Runtime/Core/GfxApi/GfxApiMethodList.inl"
#undef GfxApiMethod
    private:
#if DEBUG_DX_OBJECT_LEAK_ON_QUIT
        void ReportAllLivingObjects();
#endif
    };


    /*
    * DX12Module implementations
    */

    DX12Module::DX12Module(const EngineInitArgMap& args)
    {
        (void)args;
    }

    void DX12Module::Initialize(const EngineInitArgMap& args)
    {
        MemoryModule& mm = MemoryModule::Get();
        mm.Retain();

        CheckDebug(!gDX12GlobalState.Initialized);
        gDX12GlobalState.Initialize();

        Module::Initialize(args);
        CheckAlways(gGfxApiModule == nullptr);
        gGfxApiModule = this;
    }

    void DX12Module::StopThreads()
    {}

    void DX12Module::Finalize()
    {
        Module::Finalizing();
        if (GetUserCount() > 0)
            return;

        CheckAlways(gGfxApiModule != nullptr);
        gGfxApiModule = nullptr;

        gDX12GlobalState.Finalize();

        MemoryModule& mm = MemoryModule::Get();
        Module::Finalize();
        mm.Release();

#if DEBUG_DX_OBJECT_LEAK_ON_QUIT
        ReportAllLivingObjects();
#endif
    }


    /*
    * GfxApiMethod implementations
    */
    GfxApiBufferRef DX12Module::CreateBuffer(const GfxApiBufferDesc& desc)
    {
        (void)desc;
        //return GfxApiBufferRef(OMNI_NEW(MemoryKind::GfxApi)DX12SwapChain(desc));
        return {};
    }

    GfxApiTextureRef DX12Module::CreateTexture(const GfxApiTextureDesc& desc)
    {
        (void)desc;
        //return GfxApiSwapChainRef(OMNI_NEW(MemoryKind::GfxApi)DX12SwapChain(desc));
        return {};
    }

    GfxApiTextureRef DX12Module::WrapExternalTexture(const GfxApiTextureDesc& desc, void* texture)
    {
        (void)desc;
        (void)texture;
        //return GfxApiSwapChainRef(OMNI_NEW(MemoryKind::GfxApi)DX12SwapChain(desc));
        return {};
    }

    GfxApiSwapChainRef DX12Module::CreateSwapChain(const GfxApiSwapChainDesc& desc)
    {
        return GfxApiSwapChainRef(OMNI_NEW(MemoryKind::GfxApi)DX12SwapChain(desc));
    }

    GfxApiRenderPass* DX12Module::BeginRenderPass(const GfxApiRenderPassDesc& desc)
    {
        DX12RenderPass* renderPass = gDX12GlobalState.RenderPassCache.Alloc();
        renderPass->RecycleInit(desc);
        return renderPass;
    }

    void DX12Module::EndRenderPass(GfxApiRenderPass* pass)
    {
        DX12RenderPass* renderPass = static_cast<DX12RenderPass*>(pass);
        gDX12GlobalState.RenderPassCache.Free(renderPass);
    }

#if DEBUG_DX_OBJECT_LEAK_ON_QUIT
    void DX12Module::ReportAllLivingObjects()
    {
        IDXGIDebug* dxgiDebug;
        CheckSucceeded(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug)));
        CheckSucceeded(dxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL));
        //OmniDebugBreak();
        dxgiDebug->Release();
    }
#endif
     
    /*
    * DX12Module ctors
    */
    static Module* DX12ModuleCtor(const EngineInitArgMap& args)
    {
        return InitMemFactory<DX12Module>::New(args);
    }

    void DX12Module::Destroy()
    {
        InitMemFactory<DX12Module>::Delete((DX12Module*)this);
    }

    EXPORT_INTERNAL_MODULE(DX12Module, ModuleExportInfo(DX12ModuleCtor, false, "DX12"));
}

#endif//OMNI_WINDOWS
