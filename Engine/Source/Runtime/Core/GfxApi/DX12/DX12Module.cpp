#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Platform/WindowModule.h"
#include "Runtime/Core/System/ModuleExport.h"
#include "Runtime/Core/System/ModuleImplHelpers.h"
#include "Runtime/Core/GfxApi/GfxApiModule.h"

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

    void DX12Module::Destroy()
    {}

    void DX12Module::Initialize(const EngineInitArgMap& args)
    {
        MemoryModule& mm = MemoryModule::Get();
        mm.Retain();
        Module::Initialize(args);
    }

    void DX12Module::StopThreads()
    {}

    void DX12Module::Finalize()
    {
        Module::Finalizing();
        if (GetUserCount() > 0)
            return;

        MemoryModule& mm = MemoryModule::Get();
        Module::Finalize();
        mm.Release();
    }


    /*
    * GfxApiMethod implementations
    */
    SharedPtr<SharedObject> DX12Module::CreateResource(const GfxApiResourceDesc& desc)
    {
        (void)desc;
        return {};
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

    void DX12Module::ResizeBackbuffer(u32 width, u32 height, u32 bufferCount)
    {
        (void)width;
        (void)height;
        (void)bufferCount;
    }

    GfxApiTexture* DX12Module::GetBackBuffer(u32 index)
    {
        (void)index;
        return nullptr;
    }
    u32 DX12Module::GetBackBufferCount()
    {
        return 0;
    }

    u32 DX12Module::GetCurrentFrameIndex()
    {
        return 0;
    }

    void DX12Module::Present()
    {}


    /*
    * DX12Module ctors
    */
    static Module* DX12ModuleCtor(const EngineInitArgMap& args)
    {
        return InitMemFactory<DX12Module>::New(args);
    }
    EXPORT_INTERNAL_MODULE(DX12Module, ModuleExportInfo(DX12ModuleCtor, false, "DX12"));
}

#endif