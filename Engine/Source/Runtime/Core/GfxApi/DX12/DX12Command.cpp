#pragma once
#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/DX12/DX12Command.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineUtils.h"
#include "Runtime/Core/GfxApi/DX12/DX12Descriptor.h"
#include "Runtime/Core/GfxApi/DX12/DX12Texture.h"


namespace Omni
{

    void SetupCommandListForPass(const GfxApiRenderPass* renderPass, ID3D12GraphicsCommandList4* cmdList, bool suspend, bool resume)
    {
        D3D12_RENDER_PASS_RENDER_TARGET_DESC rtDescs[MaxMRTCount] = {};
        D3D12_RENDER_PASS_DEPTH_STENCIL_DESC dsDesc = {};
        D3D12_RENDER_PASS_FLAGS renderPassFlags = D3D12_RENDER_PASS_FLAG_NONE | D3D12_RENDER_PASS_FLAG_NONE;

        u32 mrtCount = 0;
        for (u32 iMRT = 0; iMRT < MaxMRTCount; ++iMRT)
        {
            GfxApiTexture* tex = renderPass->RenderTargets[iMRT].Texture;
            D3D12_RENDER_PASS_RENDER_TARGET_DESC& rtDesc = rtDescs[iMRT];
            if (tex == nullptr)
            {
                rtDesc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
                rtDesc.cpuDescriptor = NullCPUDescriptorHandle;
                rtDesc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;
                continue;
            }
            
            rtDesc.cpuDescriptor = ToCPUDescriptorHandle(static_cast<DX12Texture*>(tex)->GetCPUDescriptor());
            rtDesc.BeginningAccess.Type =
                Any(renderPass->RenderTargets[iMRT].Action & GfxApiLoadStoreActions::Load) ? D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE :
                Any(renderPass->RenderTargets[iMRT].Action & GfxApiLoadStoreActions::Clear) ? D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR : D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
            mrtCount = iMRT + 1;
        }
        if (suspend)
            renderPassFlags |= D3D12_RENDER_PASS_FLAG_SUSPENDING_PASS;
        if (resume)
            renderPassFlags |= D3D12_RENDER_PASS_FLAG_RESUMING_PASS;
        cmdList->BeginRenderPass(mrtCount, rtDescs, &dsDesc, renderPassFlags);
    }

    void CloseCommandListForPass(const GfxApiRenderPass* renderPass, ID3D12GraphicsCommandList4* cmdList)
    {
        (void)renderPass;
        cmdList->EndRenderPass();
        CheckDX12(cmdList->Close());
    }

    void DX12DrawRenderPass(GfxApiRenderPass* renderPass, GfxApiGpuEventRef* doneEvent)
    {
        (void)doneEvent;
        ID3D12CommandAllocator* allocator = gDX12GlobalState.DirectCommandAllocatorCache.Alloc();
        ID3D12GraphicsCommandList4* cmdList = gDX12GlobalState.DirectCommandListCache.Alloc();

        gDX12GlobalState.TimelineManager->AddBatchEvent(
            GfxApiQueueType::GraphicsQueue,
            TimelineHelpers::CreateRecycleCB(TimelineHelpers::RecycleDirectCommandAllocator, allocator));
        CheckDX12(cmdList->Reset(allocator, nullptr));
        SetupCommandListForPass(renderPass, cmdList, false, false);
        CloseCommandListForPass(renderPass, cmdList);

        ID3D12CommandList* cmd = cmdList;
        gDX12GlobalState.D3DGraphicsCommandQueue->ExecuteCommandLists(1, &cmd);
        CheckDX12(cmdList->Reset(nullptr, nullptr));
        gDX12GlobalState.DirectCommandListCache.Free(cmdList);

        delete renderPass;
    }
}


#endif//OMNI_WINDOWS
