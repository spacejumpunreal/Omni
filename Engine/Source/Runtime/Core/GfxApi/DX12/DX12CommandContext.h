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
            void* operator new(size_t size);
            void* CreateObject() override;
            void DestroyObject(void* obj) override;
            void RecycleCleanup(void* obj) override;
        };
    public:
        DX12RenderPass();
        ~DX12RenderPass();

        GfxApiCommandContext* BeginContext() override { return nullptr; };
        void EndContext(GfxApiCommandContext* ctx) override { (void)ctx; };
    private:
        PMRVector<ID3D12CommandList*>   mCommandLists;
    };
}

#endif//OMNI_WINDOWS
