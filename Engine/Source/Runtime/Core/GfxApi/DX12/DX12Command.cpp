#pragma once
#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Memory/HandleObjectPoolImpl.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/DX12/DX12Command.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12CommandUtils.h"
#include "Runtime/Core/GfxApi/DX12/DX12Descriptor.h"
#include "Runtime/Core/GfxApi/DX12/DX12Texture.h"
#include "Runtime/Core/GfxApi/DX12/DX12Buffer.h"
#include "Runtime/Core/GfxApi/DX12/d3dx12.h"

namespace Omni
{

static void SetupCommandListForPass(const GfxApiRenderPass*     renderPass,
                                    ID3D12GraphicsCommandList4* cmdList,
                                    bool                        suspend,
                                    bool                        resume)
{

    D3D12_RENDER_PASS_RENDER_TARGET_DESC  rtDescs[kMaxMRTCount] = {};
    D3D12_RENDER_PASS_DEPTH_STENCIL_DESC  dsDesc = {};
    D3D12_RENDER_PASS_RENDER_TARGET_DESC* pRTDescs = rtDescs;
    D3D12_RENDER_PASS_DEPTH_STENCIL_DESC* pDSDesc = &dsDesc;
    D3D12_RENDER_PASS_FLAGS               renderPassFlags = D3D12_RENDER_PASS_FLAG_NONE | D3D12_RENDER_PASS_FLAG_NONE;

    CD3DX12_RESOURCE_BARRIER barriers[kMaxMRTCount + 1]; // plus depth and stencil
    // RenderTarget
    u32                      mrtCount = 0;
    u32                      barrierCount = 0;
    for (u32 iMRT = 0; iMRT < kMaxMRTCount; ++iMRT)
    {
        D3D12_RENDER_PASS_RENDER_TARGET_DESC& rtDesc = rtDescs[iMRT];
        if (renderPass->RenderTargets[iMRT].Texture == GfxApiTextureRef::Null())
        {
            rtDesc.BeginningAccess.Type = D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;
            rtDesc.cpuDescriptor = NullCPUDescriptorHandle;
            rtDesc.EndingAccess.Type = D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;
            continue;
        }
        DX12Texture* tex = gDX12GlobalState.DX12TexturePool.ToPtr(renderPass->RenderTargets[iMRT].Texture);

        rtDesc.cpuDescriptor = ToCPUDescriptorHandle(tex->GetCPUDescriptor());
        rtDesc.BeginningAccess.Type = Any(renderPass->RenderTargets[iMRT].Action & GfxApiLoadStoreActions::Load)
                                          ? D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE
                                      : Any(renderPass->RenderTargets[iMRT].Action & GfxApiLoadStoreActions::Clear)
                                          ? D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR
                                          : D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_NO_ACCESS;

        if (Any(renderPass->RenderTargets[iMRT].Action & GfxApiLoadStoreActions::Clear))
        {
            rtDesc.BeginningAccess.Clear.ClearValue.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
            Vector4* pv4 = (Vector4*)rtDesc.BeginningAccess.Clear.ClearValue.Color;
            *pv4 = std::get<Vector4>(renderPass->RenderTargets[iMRT].ClearValue);
        }
        rtDesc.EndingAccess.Type = Any(renderPass->RenderTargets[iMRT].Action & GfxApiLoadStoreActions::Store)
                                       ? D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE
                                   : Any(renderPass->RenderTargets[iMRT].Action & GfxApiLoadStoreActions::Discard)
                                       ? D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD
                                       : D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;
        if (tex->EmitBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET, barriers + barrierCount))
            ++barrierCount;

        mrtCount = iMRT + 1;
    }
    if (mrtCount == 0)
    {
        pRTDescs = nullptr;
    }
    // Depth & stencil
    if (renderPass->Depth.Texture == GfxApiTextureRef::Null())
    {
        pDSDesc = nullptr;
    }
    else
    {
        NotImplemented("Barrier logic");
        dsDesc.cpuDescriptor = ToCPUDescriptorHandle(
            gDX12GlobalState.DX12TexturePool.ToPtr(renderPass->Depth.Texture)->GetCPUDescriptor());
        dsDesc.DepthBeginningAccess.Type = Any(renderPass->Depth.Action & GfxApiLoadStoreActions::Load)
                                               ? D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE
                                           : Any(renderPass->Depth.Action & GfxApiLoadStoreActions::Clear)
                                               ? D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR
                                               : D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
        if (Any(renderPass->Depth.Action & GfxApiLoadStoreActions::Clear))
        {
            dsDesc.DepthBeginningAccess.Clear.ClearValue.Format = DXGI_FORMAT_R32_FLOAT;
            dsDesc.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth =
                std::get<float>(renderPass->Depth.ClearValue);
        }
        dsDesc.DepthEndingAccess.Type = Any(renderPass->Depth.Action & GfxApiLoadStoreActions::Store)
                                            ? D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE
                                        : Any(renderPass->Depth.Action & GfxApiLoadStoreActions::Discard)
                                            ? D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD
                                            : D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;

        dsDesc.StencilBeginningAccess.Type = Any(renderPass->Stencil.Action & GfxApiLoadStoreActions::Load)
                                                 ? D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_PRESERVE
                                             : Any(renderPass->Stencil.Action & GfxApiLoadStoreActions::Clear)
                                                 ? D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_CLEAR
                                                 : D3D12_RENDER_PASS_BEGINNING_ACCESS_TYPE_DISCARD;
        if (Any(renderPass->Stencil.Action & GfxApiLoadStoreActions::Clear))
        {
            dsDesc.DepthBeginningAccess.Clear.ClearValue.Format = DXGI_FORMAT_R8_UINT;
            dsDesc.DepthBeginningAccess.Clear.ClearValue.DepthStencil.Depth =
                std::get<byte>(renderPass->Stencil.ClearValue);
        }
        dsDesc.DepthEndingAccess.Type = Any(renderPass->Stencil.Action & GfxApiLoadStoreActions::Store)
                                            ? D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_PRESERVE
                                        : Any(renderPass->Stencil.Action & GfxApiLoadStoreActions::Discard)
                                            ? D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_DISCARD
                                            : D3D12_RENDER_PASS_ENDING_ACCESS_TYPE_NO_ACCESS;
    }
    if (barrierCount != 0)
    {
        cmdList->ResourceBarrier(barrierCount, barriers);
    }

    if (suspend)
        renderPassFlags |= D3D12_RENDER_PASS_FLAG_SUSPENDING_PASS;
    if (resume)
        renderPassFlags |= D3D12_RENDER_PASS_FLAG_RESUMING_PASS;
    cmdList->BeginRenderPass(mrtCount, pRTDescs, pDSDesc, renderPassFlags);
}

static void CloseCommandListForPass(const GfxApiRenderPass* renderPass, ID3D12GraphicsCommandList4* cmdList)
{
    (void)renderPass;
    cmdList->EndRenderPass();
    CheckDX12(cmdList->Close());
}

void DX12DrawRenderPass(GfxApiRenderPass* renderPass)
{
    ID3D12GraphicsCommandList4* cmdList = SetupCommandList(GfxApiQueueType::GraphicsQueue);
    SetupCommandListForPass(renderPass, cmdList, false, false);
    CloseCommandListForPass(renderPass, cmdList);
    ID3D12CommandList* cmd = cmdList;
    gDX12GlobalState.Singletons.D3DQueues[(u32)GfxApiQueueType::GraphicsQueue]->ExecuteCommandLists(1, &cmd);
    gDX12GlobalState.CommandListCache[(u32)GfxApiQueueType::GraphicsQueue].Free(cmdList);

    delete renderPass;
}

void DX12CopyBlitPass(GfxApiBlitPass* blitPass)
{
    ID3D12GraphicsCommandList4* cmdListPrelude = SetupCommandList(GfxApiQueueType::CopyQueue);
    ID3D12GraphicsCommandList4* cmdListCopy = SetupCommandList(GfxApiQueueType::CopyQueue);

    PMRVector<D3D12_RESOURCE_BARRIER> barriersToCopyDest(MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApiTmp));

    barriersToCopyDest.reserve(blitPass->CopyBufferCmds.size());
    for (GfxApiCopyBuffer& copyBufferCmd : blitPass->CopyBufferCmds)
    {
        DX12Buffer* dstBuffer = gDX12GlobalState.DX12BufferPool.ToPtr(copyBufferCmd.Dst);
        barriersToCopyDest.emplace_back();
        if (!dstBuffer->EmitBarrier(D3D12_RESOURCE_STATE_COPY_DEST, &barriersToCopyDest.back()))
            barriersToCopyDest.pop_back();
        DX12Buffer* srcBuffer = gDX12GlobalState.DX12BufferPool.ToPtr(copyBufferCmd.Src);
        cmdListCopy->CopyBufferRegion(dstBuffer->GetResource(),
                                      copyBufferCmd.DstOffset,
                                      srcBuffer->GetResource(),
                                      copyBufferCmd.SrcOffset,
                                      copyBufferCmd.Bytes);
    }
    cmdListPrelude->ResourceBarrier((u32)barriersToCopyDest.size(), barriersToCopyDest.data());

    CheckDX12(cmdListPrelude->Close());
    CheckDX12(cmdListCopy->Close());
    ID3D12CommandList* cmdLists[] = {cmdListPrelude, cmdListCopy};
    gDX12GlobalState.Singletons.D3DQueues[(u8)GfxApiQueueType::CopyQueue]->ExecuteCommandLists(2, cmdLists);
    
    auto& cmdListCache = gDX12GlobalState.CommandListCache[(u32)GfxApiQueueType::CopyQueue];
    cmdListCache.Free(cmdListPrelude);
    cmdListCache.Free(cmdListCopy);

    delete blitPass;
}

} // namespace Omni

#endif // OMNI_WINDOWS
