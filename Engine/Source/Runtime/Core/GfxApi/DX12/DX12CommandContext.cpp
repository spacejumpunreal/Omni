#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12CommandContext.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineUtils.h"
#include "Runtime/Core/GfxApi/DX12/DX12Texture.h"
#include "Runtime/Core/GfxApi/DX12/DX12Descriptor.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
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

    //DX12RenderPass::CacheFactory
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
        auto* pass = static_cast<DX12RenderPass*>(obj);
        pass->mPhases.clear();
    }
    
    //DX12RenderPass
    DX12RenderPass::DX12RenderPass()
        : mPhases(MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApi))
    {}
    DX12RenderPass::~DX12RenderPass()
    {}

    void AcquireRenderPassTextures(const GfxApiRenderPassDesc& desc)
    {
        for (u32 mrtCount = 0; mrtCount < MaxMRTCount; ++mrtCount)
        {
            GfxApiTexture* tex = desc.Color[mrtCount].Texture;
            if (tex)
                tex->Acquire();
            gDX12GlobalState.TimelineManager->AddBatchEvent(
                GfxApiQueueType::GraphicsQueue,
                TimelineHelpers::CreateRecycleCB(TimelineHelpers::ReleaseSharedObject, (SharedObject*)tex));
        }
    }

    void SetupCommandListForPass(const GfxApiRenderPassDesc& desc, ID3D12GraphicsCommandList4* cmdLst, bool suspend, bool resume) //TODO add flags here to specify order in render pass)
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
                Any(desc.Color[mrtCount].Action & GfxApiLoadStoreActions::Load) ? D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE :
                Any(desc.Color[mrtCount].Action & GfxApiLoadStoreActions::Clear) ? D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR : D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
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
        mPhases.resize(desc.StageCount);
        mDesc = desc;
        AcquireRenderPassTextures(desc);
    }

    void DX12RenderPass::CommitRenderPass()
    {
        CheckDebug(mPhases[0].size() != 0, "1st phase in render pass shall not be missing");
        for (CommandListVec& phase : mPhases)
        {
            if (phase.size() == 0)
                continue;
            ID3D12CommandList** lists = (ID3D12CommandList**)phase.data();
            gDX12GlobalState.D3DGraphicsCommandQueue->ExecuteCommandLists((u32)phase.size(), lists);
            for (ID3D12GraphicsCommandList4* list : phase)
            {
                gDX12GlobalState.DirectCommandListCache.Free(list);
            }
        }
    }

    GfxApiRenderCommandContext* DX12RenderPass::BeginContext(u32 phase)
    {
        DX12RenderCommandContext* ctx = gDX12GlobalState.RenderCommandContextCache.Alloc();
        ID3D12CommandAllocator* allocator = gDX12GlobalState.DirectCommandAllocatorCache.Alloc();
        ID3D12GraphicsCommandList4* cmdList = gDX12GlobalState.DirectCommandListCache.Alloc();

        gDX12GlobalState.TimelineManager->AddBatchEvent(
            GfxApiQueueType::GraphicsQueue,
            TimelineHelpers::CreateRecycleCB(TimelineHelpers::RecycleDirectCommandAllocator, allocator));
        CheckGfxApi(cmdList->Reset(allocator, nullptr));
        SetupCommandListForPass(mDesc, cmdList, phase == 0, phase == mDesc.StageCount - 1);
        ctx->RecycleInit(cmdList);
        mPhases[phase].push_back(cmdList);
        return ctx;
    }
    void DX12RenderPass::EndContext(GfxApiRenderCommandContext* ctx)
    {
        DX12RenderCommandContext* _ctx = static_cast<DX12RenderCommandContext*>(ctx);
        _ctx->EndEncoding();
        gDX12GlobalState.RenderCommandContextCache.Free(_ctx);
    }

    //DX12RenderCommandContext::CacheFactory
    DX12RenderCommandContext::CacheFactory::CacheFactory()
    {}
    void* DX12RenderCommandContext::CacheFactory::CreateObject()
    {
        return OMNI_NEW(MemoryKind::GfxApi)DX12RenderCommandContext();
    }
    void DX12RenderCommandContext::CacheFactory::DestroyObject(void* obj)
    {
        auto* context = static_cast<DX12RenderCommandContext*>(obj);
        OMNI_DELETE(context, MemoryKind::GfxApi);
    }
    void DX12RenderCommandContext::CacheFactory::RecycleCleanup(void* obj)
    {
        auto* context = static_cast<DX12RenderCommandContext*>(obj);
        context->mCommandList = nullptr;
    }

    //DX12RenderCommandContext
    DX12RenderCommandContext::DX12RenderCommandContext()
        : mCommandList(nullptr)
    {}
    void DX12RenderCommandContext::RecycleInit(ID3D12GraphicsCommandList4* cmdList)
    {
        mCommandList = cmdList;
    }
    void DX12RenderCommandContext::EndEncoding()
    {
        CheckGfxApi(mCommandList->Close());
        mCommandList->Reset(nullptr, nullptr);
    }
    void DX12RenderCommandContext::Use()
    {}
}

#endif//OMNI_WINDOWS
