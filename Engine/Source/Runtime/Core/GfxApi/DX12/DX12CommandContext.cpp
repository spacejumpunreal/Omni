#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12CommandContext.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineUtils.h"

namespace Omni
{
    /**
    * forward decl
    */

    /**
    * constants
    */

    /**
    * declarations
    */

    /**
    * definitions
    */
    DX12RenderPass::CacheFactory::CacheFactory()
    {}
    void* DX12RenderPass::CacheFactory::CreateObject()
    {
        return OMNI_NEW(MemoryKind::GfxApi)DX12RenderPass();
    }
    void DX12RenderPass::CacheFactory::DestroyObject(void* obj)
    {
        auto* pass = static_cast<DX12RenderPass*>(obj);
        OMNI_DELETE(pass, MemoryKind::GfxApi);
    }
    void DX12RenderPass::CacheFactory::RecycleCleanup(void* obj)
    {
        (void)obj;
    }
    
    DX12RenderPass::DX12RenderPass()
    {}

    DX12RenderPass::~DX12RenderPass()
    {
    }

    void DX12RenderPass::RecycleInit(const GfxApiRenderPassDesc& desc)
    {
        //auto xxx = (void (*)(void*))&TimelineHelpers::RecycleCommandAllocator;
        ID3D12CommandAllocator* allocator = gDX12GlobalState.DirectCommandAllocatorCache.Alloc();
        ID3D12GraphicsCommandList4* cmdLst = gDX12GlobalState.DirectCommandListCache.Alloc();

        gDX12GlobalState.TimelineManager->AddBatchEvent(
            GfxApiQueueType::GraphicsQueue, 
            TimelineHelpers::CreateRecycleCB(&TimelineHelpers::RecycleDirectCommandAllocator, allocator));
        cmdLst->Reset(allocator, nullptr);
        
        u32 mrtCount;
        for (mrtCount = 0; mrtCount < MaxMRTCount; ++mrtCount)
        {
            if (desc.Color[mrtCount].Texture.GetRaw() == nullptr)
                break;
            desc.Color[mrtCount].Texture.GetRaw()->Acquire();
            gDX12GlobalState.TimelineManager->AddBatchEvent(
                GfxApiQueueType::GraphicsQueue,
                TimelineHelpers::CreateRecycleCB(&TimelineHelpers::RecycleDirectCommandAllocator, allocator));
        }
        //cmdLst->BeginRenderPass(mrtCount, rtDescs, dsDescs, )
    }

    
}




#endif


