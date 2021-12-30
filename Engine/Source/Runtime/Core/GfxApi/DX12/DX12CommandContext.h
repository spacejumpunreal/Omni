#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/GfxApi/GfxApiObject.h"
#include "Runtime/Core/GfxApi/GfxApiObjectCacheFactory.h"

//forward decl

namespace Omni
{
    class DX12RenderPass: public GfxApiRenderPass
    {
    public:
        struct CacheFactory : public GfxApiObjectCacheFactory<DX12RenderPass, CacheFactory>
        {
            CacheFactory();
            void* CreateObject() override;
            void DestroyObject(void* obj) override;
            void RecycleCleanup(void* obj) override;
        };
    public:
        DX12RenderPass();
        ~DX12RenderPass();
        void RecycleInit(const GfxApiRenderPassDesc& desc);
        GfxApiRenderCommandContext* BeginContext() override;
        void EndContext(GfxApiRenderCommandContext* ctx) override;
    private:
        using CommandListVec = PMRVector<ID3D12CommandList*>;
        PMRVector<CommandListVec>   mCommandLists;
    };

    class DX12RenderCommandContext : public GfxApiRenderCommandContext
    {
    public:
        void Use() override {}
    private:
        ID3D12CommandList* mCommandList;
    };
}

#endif//OMNI_WINDOWS
