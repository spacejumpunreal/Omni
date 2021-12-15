#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Platform/WindowModule.h"
#include "Runtime/Core/System/ModuleExport.h"
#include "Runtime/Core/System/ModuleImplHelpers.h"
#include "Runtime/Core/GfxApi/GfxApiModule.h"
#include "Runtime/Core/GfxApi/DX12/DX12Context.h"
#include "Runtime/Core/GfxApi/DX12/DX12SwapChain.h"

#include <d3d12.h>


namespace Omni
{
    //forward decls
    // 
    //declarations
    class DX12Module : public GfxApiModule
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

        CheckDebug(!gDX12Context.Initialized);
        gDX12Context.Initialize();

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

        gDX12Context.Finalize();

        MemoryModule& mm = MemoryModule::Get();
        Module::Finalize();
        mm.Release();
    }


    /*
    * GfxApiMethod implementations
    */
    SharedPtr<SharedObject> DX12Module::CreateGfxApiObject(const GfxApiObjectDesc& desc)
    {
        SharedObject* ret = nullptr;
        switch (desc.Type)
        {
        case GfxApiObjectType::Swapchain:
            ret = OMNI_NEW(MemoryKind::GfxApi)DX12SwapChain(static_cast<const GfxApiSwapChainDesc&>(desc));
            break;
        default:
            NotImplemented();
            break;
        }
        return ret;
    }

    void DX12Module::UpdateSwapChain(GfxApiSwapChain& swapChain)
    {
        (void)swapChain;
    }

    GfxApiRenderPass* DX12Module::BeginRenderPass(const GfxApiRenderPassDesc& desc)
    {
        (void)desc;
        return nullptr;
    }

    void DX12Module::EndRenderPass(const GfxApiRenderPass* desc)
    {
        (void)desc;
    }

    GfxApiContext* DX12Module::BeginContext(const GfxApiContextDesc& desc)
    {
        (void)desc;
        return nullptr;
    }
    
    void DX12Module::EndContext(GfxApiContext* context)
    {
        (void)context;
    }


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