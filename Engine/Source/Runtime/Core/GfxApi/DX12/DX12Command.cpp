#pragma once
#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Memory/HandleObjectPoolImpl.h"
#include "Runtime/Base/Memory/MemoryArena.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/DX12/DX12Command.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12PSOManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12DescriptorManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12DeleteManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12CommandUtils.h"
#include "Runtime/Core/GfxApi/DX12/DX12Descriptor.h"
#include "Runtime/Core/GfxApi/DX12/DX12Texture.h"
#include "Runtime/Core/GfxApi/DX12/DX12Buffer.h"
#include "Runtime/Core/GfxApi/DX12/DX12Shader.h"
#include "Runtime/Core/GfxApi/DX12/d3dx12.h"

namespace Omni
{
/*
 * definitions
 */

struct DX12RenderStageCommon
{
public:
    DX12RenderStageCommon(const GfxApiRenderPass* renderPass);

public:
    D3D12_CPU_DESCRIPTOR_HANDLE RTDescriptions[kMaxMRTCount];
    D3D12_CPU_DESCRIPTOR_HANDLE DSDescriptor;
    u8                          RTCount;

    D3D12_RECT     DefaultScissors[kMaxMRTCount];
    D3D12_VIEWPORT DefaultViewports[kMaxMRTCount];
    GfxApiFormat   RTFormats[kMaxMRTCount];
};

static void ToDX12Viewports(const GfxApiViewport viewports[], D3D12_VIEWPORT dx12Viewports[], u32 count)
{
    static_assert(sizeof(GfxApiViewport) == sizeof(D3D12_VIEWPORT));
    memcpy(dx12Viewports, viewports, sizeof(D3D12_VIEWPORT) * count);
}

DX12RenderStageCommon::DX12RenderStageCommon(const GfxApiRenderPass* renderPass)
{
    ZeroFill(*this);
    for (RTCount = 0;; ++RTCount)
    {
        auto texHandle = renderPass->RenderTargets[RTCount].Texture;
        if (texHandle == GfxApiTextureRef::Null())
            break;
        DX12Texture* tex = gDX12GlobalState.DX12TexturePool.ToPtr(texHandle);
        RTDescriptions[RTCount] =
            ToCPUDescriptorHandle(gDX12GlobalState.DX12TexturePool.ToPtr(texHandle)->GetCPUDescriptor());
        RTFormats[RTCount] = tex->GetDesc().Format;

        DefaultScissors[RTCount].left = DefaultScissors[RTCount].top = 0;
        DefaultScissors[RTCount].right = tex->GetDesc().Width;
        DefaultScissors[RTCount].bottom = tex->GetDesc().Height;

        DefaultViewports[RTCount].TopLeftX = DefaultViewports[RTCount].TopLeftY = 0;
        DefaultViewports[RTCount].Width = (float)tex->GetDesc().Width;
        DefaultViewports[RTCount].Height = (float)tex->GetDesc().Height;
        DefaultViewports[RTCount].MinDepth = 0;
        DefaultViewports[RTCount].MaxDepth = 1;
    }
    if (renderPass->Depth.Texture != GfxApiTextureRef::Null())
        NotImplemented();
}

static bool EncodeDrawcalls(const DX12RenderStageCommon& stageCommon,
    const GfxApiDrawcall                                 drawcalls[],
    u32                                                  drawcallCount,
    ID3D12GraphicsCommandList4*                          gCmdList)
{
    DX12PSOKey psoKey;
    ZeroFill(psoKey);
    auto& stk = MemoryModule::Get().GetThreadScratchStack();
    auto  stkScope = stk.PushScope();
    bool  valid = false;

    DX12PSOSignature* rootSig;

    memcpy(psoKey.RTFormats, stageCommon.RTFormats, sizeof(stageCommon.RTFormats));
    psoKey.RTCount = stageCommon.RTCount;

    gCmdList->RSSetScissorRects(stageCommon.RTCount, stageCommon.DefaultScissors);
    gCmdList->OMSetRenderTargets(stageCommon.RTCount, stageCommon.RTDescriptions, FALSE, nullptr);

    DX12DescriptorTmpAllocator* srvDescPool =
        new DX12DescriptorTmpAllocator(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 16 * 1024);

    for (u32 idc = 0; idc < drawcallCount; ++idc)
    {
        const GfxApiDrawcall& dc = drawcalls[idc];
        // IA
        {
            if (dc.IndexBuffer != GfxApiBufferRef::Null())
            {
                DX12Buffer*             indexBuffer = gDX12GlobalState.DX12BufferPool.ToPtr(dc.IndexBuffer);
                D3D12_INDEX_BUFFER_VIEW ibv;
                ibv.BufferLocation = indexBuffer->GetResource()->GetGPUVirtualAddress();
                ibv.Format = dc.Is32BitIndexBuffer ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
                ibv.SizeInBytes = indexBuffer->GetDesc().Size;
                gCmdList->IASetIndexBuffer(&ibv);
            }
            else
                gCmdList->IASetIndexBuffer(nullptr);
            gCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        }
        // PSO
        {
            rootSig = gDX12GlobalState.DX12PSOSignaturePool.ToPtr(dc.PSOParams.Signature);
            gCmdList->SetGraphicsRootSignature(rootSig->GetRootSignature());
            psoKey.Params = dc.PSOParams;
            ID3D12PipelineState* pso = gDX12GlobalState.PSOManager->GetOrCreatePSO(psoKey);
            gCmdList->SetPipelineState(pso);
        }
        // Binding
        {
            ID3D12DescriptorHeap* srvHeap = srvDescPool->EnsureSpace(64); // this is the limit that I can think of
            gCmdList->SetDescriptorHeaps(1, &srvHeap);
            u32                      paramCount = rootSig->GetRootParamCount();
            const GfxApiBindingSlot* params = rootSig->GetRootParamDescs();
            for (u32 iParam = 0; iParam < paramCount; ++iParam)
            {
                u32 bindingGroupIdx = params[iParam].FromBindingGroup;
                switch (params[iParam].Type)
                {
                case GfxApiBindingType::ConstantBuffer:
                {
                    auto cbv = dc.BindingGroups[bindingGroupIdx]->ConstantBuffer;
                    CheckDebug(cbv.Buffer != GfxApiBufferRef::Null());
                    auto gpuAddr =
                        gDX12GlobalState.DX12BufferPool.ToPtr(cbv.Buffer)->GetResource()->GetGPUVirtualAddress();
                    gCmdList->SetGraphicsRootConstantBufferView(iParam, gpuAddr + cbv.Offset);
                    break;
                }
                case GfxApiBindingType::Buffers:
                {
                    auto  bufferCnt = dc.BindingGroups[bindingGroupIdx]->BufferCount;
                    auto  buffers = dc.BindingGroups[bindingGroupIdx]->Buffers;
                    auto& bufferPool = gDX12GlobalState.DX12BufferPool;
                    CheckDebug(bufferCnt != 0);

                    D3D12_CPU_DESCRIPTOR_HANDLE cpuStart;
                    D3D12_GPU_DESCRIPTOR_HANDLE gpuStart;
                    srvDescPool->Alloc(bufferCnt, cpuStart, gpuStart);

                    auto                         scope = stk.PushScope();
                    D3D12_CPU_DESCRIPTOR_HANDLE* srcHandles = stk.AllocArray<D3D12_CPU_DESCRIPTOR_HANDLE>(bufferCnt);
                    u32*                         srcRangeLengths = stk.AllocArray<u32>(bufferCnt);
                    memset(srcRangeLengths, 1, bufferCnt);
                    for (u32 iBuffer = 0; iBuffer < bufferCnt; ++iBuffer)
                    {
                        DX12Descriptor d = bufferPool.ToPtr(buffers[iBuffer])->GetSRVDescriptor();
                        srcHandles[iBuffer] = ToCPUDescriptorHandle(d);
                    }
                    u32 one = 1;
                    gDX12GlobalState.Singletons.D3DDevice->CopyDescriptors(1,
                        &cpuStart,
                        &one,
                        bufferCnt,
                        srcHandles,
                        srcRangeLengths,
                        D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
                    gCmdList->SetGraphicsRootDescriptorTable(iParam, gpuStart);

                    break;
                }

                case GfxApiBindingType::Textures:
                case GfxApiBindingType::UAVs:
                case GfxApiBindingType::Samplers:
                default:
                    NotImplemented();
                    break;
                }
            }
            gCmdList->OMSetStencilRef(dc.StencilRef);
        }
        // draw states
        {
            const D3D12_VIEWPORT* vps = stageCommon.DefaultViewports;
            D3D12_VIEWPORT        tmpViewports[kMaxMRTCount];
            if (dc.Viewports)
            {
                ToDX12Viewports(dc.Viewports, tmpViewports, stageCommon.RTCount);
                vps = tmpViewports;
            }
            gCmdList->RSSetViewports(stageCommon.RTCount, vps);
        }
        // call draw
        if (dc.IsIndirect)
        {
            NotImplemented();
        }
        else
        {
            if (dc.IndexBuffer != GfxApiBufferRef::Null())
            {
                auto& dda = dc.DrawArgs.DirectDrawArgs;
                gCmdList->DrawIndexedInstanced(dda.IndexCount,
                    dda.InstanceCount,
                    dda.FirstIndex,
                    dda.BaseVertex,
                    dda.BaseInstance);
                valid = true;
            }
            else
            {
                NotImplemented();
            }
        }
    }
    gDX12GlobalState.DeleteManager->AddForDelete(srvDescPool, 1 << u32(GfxApiQueueType::GraphicsQueue));

    return valid;
}

static bool EncodePassPrelude(const GfxApiRenderPass* renderPass, ID3D12GraphicsCommandList4* cmdList)
{
    CD3DX12_RESOURCE_BARRIER    barriers[kMaxMRTCount + 2]; // plus depth and stencil
    D3D12_CPU_DESCRIPTOR_HANDLE rtToClear[kMaxMRTCount];
    Vector4                     rtClearColors[kMaxMRTCount];

    bool valid = false;
    u32  barrierCount = 0;
    u32  clearCount = 0;

    for (u32 iRT = 0; iRT < kMaxMRTCount; ++iRT)
    {
        auto& cfg = renderPass->RenderTargets[iRT];
        if (cfg.Texture == GfxApiTextureRef::Null())
            break;
        DX12Texture* tex = gDX12GlobalState.DX12TexturePool.ToPtr(renderPass->RenderTargets[iRT].Texture);
        if (tex->EmitBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET, barriers + barrierCount))
            ++barrierCount;
        if (Any(cfg.Action & (GfxApiLoadStoreActions::Clear | GfxApiLoadStoreActions::Discard)))
        {
            rtToClear[clearCount] = ToCPUDescriptorHandle(tex->GetCPUDescriptor());
            rtClearColors[clearCount] = std::get<Vector4>(cfg.ClearValue);
            ++clearCount;
        }
    }

    if (renderPass->Depth.Texture != GfxApiTextureRef::Null() ||
        renderPass->Stencil.Texture != GfxApiTextureRef::Null())
    {
        NotImplemented();
        // TODO: cmdList->ClearDepthStencilView()
    }

    if (barrierCount > 0)
    {
        valid = true;
        cmdList->ResourceBarrier(barrierCount, barriers);
    }

    for (u32 iMRT = 0; iMRT < clearCount; ++iMRT)
    {
        cmdList->ClearRenderTargetView(rtToClear[iMRT], (float*)&rtClearColors[iMRT], 0, nullptr);
        valid = true;
    }
    return valid;
}

void DX12DrawRenderPass(const GfxApiRenderPass* renderPass)
{
    u32                 stageCount = renderPass->GetStageCount();
    auto&               stk = MemoryModule::Get().GetThreadScratchStack();
    auto                stkScope = stk.PushScope();
    ID3D12CommandList** allCmdLists = stk.AllocArray<ID3D12CommandList*>(stageCount);
    ID3D12CommandList** validCmdLists = stk.AllocArray<ID3D12CommandList*>(stageCount);
    u32                 allListCount = 0;
    u32                 validListCount = 0;
    bool                valid;

    ID3D12GraphicsCommandList4* cmdList;
    ID3D12CommandQueue*         gQueue = gDX12GlobalState.Singletons.D3DQueues[(u32)GfxApiQueueType::GraphicsQueue];

    { // barrier and clear
        cmdList = SetupCommandList(GfxApiQueueType::GraphicsQueue);
        valid = EncodePassPrelude(renderPass, cmdList);
        CheckDX12(cmdList->Close());
        if (valid)
        {
            ID3D12CommandList* cl = cmdList;
            gQueue->ExecuteCommandLists(1, &cl);
        }
        gDX12GlobalState.CommandListCache[(u32)GfxApiQueueType::GraphicsQueue].Free(cmdList);
    }

    DX12RenderStageCommon stageCommon(renderPass);
    for (u32 iStage = 0; iStage < renderPass->PhaseCount; ++iStage)
    {
        for (auto* stage = renderPass->PhaseArray[iStage]; stage != nullptr;
             stage = (GfxApiRenderPassStage*)stage->Next)
        {
            cmdList = SetupCommandList(GfxApiQueueType::GraphicsQueue);
            allCmdLists[allListCount++] = cmdList;
            valid = EncodeDrawcalls(stageCommon, stage->Drawcalls, stage->DrawcallCount, cmdList);
            CheckDX12(cmdList->Close());
            if (valid)
                validCmdLists[validListCount++] = (ID3D12CommandList*)cmdList;
        }
    }
    if (validListCount > 0)
        gQueue->ExecuteCommandLists(validListCount, validCmdLists);

    for (u32 iCmdList = 0; iCmdList < allListCount; ++iCmdList)
    {
        auto glist = (ID3D12GraphicsCommandList4*)allCmdLists[iCmdList];
        gDX12GlobalState.CommandListCache[(u32)GfxApiQueueType::GraphicsQueue].Free(glist);
    }
}

void DX12CopyBlitPass(const GfxApiBlitPass* blitPass)
{
    ID3D12GraphicsCommandList4* cmdListPrelude = SetupCommandList(GfxApiQueueType::CopyQueue);
    ID3D12GraphicsCommandList4* cmdListCopy = SetupCommandList(GfxApiQueueType::CopyQueue);

    PMRVector<D3D12_RESOURCE_BARRIER> barriersToCopyDest(MemoryModule::Get().GetPMRAllocator(MemoryKind::GfxApiTmp));

    barriersToCopyDest.reserve(blitPass->CopyBufferCmdCount);
    // for (GfxApiCopyBuffer& copyBufferCmd : blitPass->CopyBufferCmds)
    for (u32 iCmd = 0; iCmd < blitPass->CopyBufferCmdCount; ++iCmd)
    {
        GfxApiCopyBuffer& copyBufferCmd = blitPass->CopyBufferCmds[iCmd];
        DX12Buffer*       dstBuffer = gDX12GlobalState.DX12BufferPool.ToPtr(copyBufferCmd.Dst);
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
}

} // namespace Omni

#endif // OMNI_WINDOWS
