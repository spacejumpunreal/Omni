#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12CommandContext.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineUtils.h"
#include "Runtime/Core/GfxApi/DX12/DX12Texture.h"
#include "Runtime/Core/GfxApi/DX12/DX12Descriptor.h"
#include <d3d12.h>

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


    FORCEINLINE void SetupRenderPass(const GfxApiRenderPassDesc& desc, ID3D12GraphicsCommandList4* cmdLst, bool suspend, bool resume) //TODO add flags here to specify order in render pass)
    {
        D3D12_RENDER_PASS_RENDER_TARGET_DESC rtDescs[MaxMRTCount] = {};
        D3D12_RENDER_PASS_DEPTH_STENCIL_DESC dsDesc = {};

        for (u32 mrtCount = 0; mrtCount < MaxMRTCount; ++mrtCount)
        {
            GfxApiTexture* tex = desc.Color[mrtCount].Texture;
            D3D12_RENDER_PASS_RENDER_TARGET_DESC& rtDesc = rtDescs[mrtCount];
            if (tex == nullptr)
            {
                rtDesc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
                rtDesc.cpuDescriptor = NullCPUDescriptorHandle;
                rtDesc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;
                continue;
            }
            
            rtDesc.cpuDescriptor = ToCPUDescriptorHandle(static_cast<DX12Texture*>(tex)->GetCPUDescriptor());
            rtDesc.BeginningAccess.Type =
                (desc.Color[mrtCount].Action & GfxApiLoadStoreActions::Load) ? D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE :
                (desc.Color[mrtCount].Action & GfxApiLoadStoreActions::Clear) ? D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR : D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;


            tex->Acquire();
            gDX12GlobalState.TimelineManager->AddBatchEvent(
                GfxApiQueueType::GraphicsQueue,
                TimelineHelpers::CreateRecycleCB(TimelineHelpers::ReleaseSharedObject, (SharedObject*)tex));
            D3D12_RENDER_PASS_FLAGS renderPassFlags = D3D12_RENDER_PASS_FLAG_NONE | D3D12_RENDER_PASS_FLAG_NONE;
            if (suspend)
                renderPassFlags |= D3D12_RENDER_PASS_FLAG_SUSPENDING_PASS;
            if (resume)
                renderPassFlags |= D3D12_RENDER_PASS_FLAG_RESUMING_PASS;
            cmdLst->BeginRenderPass(mrtCount, rtDescs, &dsDesc, renderPassFlags);
        }
    }

    void DX12RenderPass::RecycleInit(const GfxApiRenderPassDesc& desc)
    {
        CheckDebug(desc.StageCount > 0, "at least 1 stage in a render pass");
        mCommandLists.resize(desc.StageCount);
#if 0
        ID3D12CommandAllocator* allocator = gDX12GlobalState.DirectCommandAllocatorCache.Alloc();
        ID3D12GraphicsCommandList4* cmdLst = gDX12GlobalState.DirectCommandListCache.Alloc();

        gDX12GlobalState.TimelineManager->AddBatchEvent(
            GfxApiQueueType::GraphicsQueue, 
            TimelineHelpers::CreateRecycleCB(TimelineHelpers::RecycleDirectCommandAllocator, allocator));
        cmdLst->Reset(allocator, nullptr);
        SetupRenderPass(desc, cmdLst);
#endif
    }

    GfxApiRenderCommandContext* DX12RenderPass::BeginContext()
    {}
    void DX12RenderPass::EndContext(GfxApiRenderCommandContext* ctx)
    {}
}

#endif//OMNI_WINDOWS
