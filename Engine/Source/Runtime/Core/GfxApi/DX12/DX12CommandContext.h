#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/GfxApi/GfxApiObject.h"
#include "Runtime/Core/GfxApi/GfxApiObjectCacheFactory.h"

//forward decl
struct ID3D12GraphicsCommandList4;


namespace Omni
{
    class DX12RenderPass: public GfxApiRenderPass
    {
    public:
        DECLARE_GFXAPI_OBJECT_CACHE_FACTORY_BEGIN(CacheFactory, DX12RenderPass)
        DECLARE_GFXAPI_OBJECT_CACHE_FACTORY_END()

    public:
        DX12RenderPass();
        ~DX12RenderPass();
        void RecycleInit(const GfxApiRenderPassDesc& desc);
        void CommitRenderPass();
        GfxApiRenderCommandContext* BeginContext(u32 phase) override;
        void EndContext(GfxApiRenderCommandContext* ctx) override;
    private:
        using CommandListVec = PMRVector<ID3D12GraphicsCommandList4*>;
        PMRVector<CommandListVec>   mPhases;
        GfxApiRenderPassDesc        mDesc;
    };

    class DX12RenderCommandContext : public GfxApiRenderCommandContext
    {
    public:
        DECLARE_GFXAPI_OBJECT_CACHE_FACTORY_BEGIN(CacheFactory, DX12RenderCommandContext)
        DECLARE_GFXAPI_OBJECT_CACHE_FACTORY_END()
    public:
        DX12RenderCommandContext();
        void RecycleInit(ID3D12GraphicsCommandList4* cmdList);
        void EndEncoding();
        void Use() override;
    private:
        ID3D12GraphicsCommandList4* mCommandList;
    };
}

#endif//OMNI_WINDOWS
