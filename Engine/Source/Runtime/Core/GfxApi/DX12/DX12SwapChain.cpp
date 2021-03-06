#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Memory/HandleObjectPoolImpl.h"
#include "Runtime/Core/GfxApi/DX12/DX12SwapChain.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12Fence.h"
#include "Runtime/Core/GfxApi/DX12/DX12Texture.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/DX12/DX12Descriptor.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12DescriptorManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineUtils.h"
#include "Runtime/Core/GfxApi/DX12/DX12CommandUtils.h"

#include "Runtime/Core/GfxApi/DX12/d3dx12.h"

namespace Omni
{
// constants
const wchar_t* BackBufferNames[] = {
    L"BackBuffer0",
    L"BackBuffer1",
    L"BackBuffer2",
};

DX12SwapChain::DX12SwapChain(const GfxApiSwapChainDesc& desc) : mDesc(desc), mDX12SwapChain(nullptr)
{
    DXGI_SWAP_CHAIN_DESC1 winDesc = {};
    winDesc.Width = mDesc.Width;
    winDesc.Height = mDesc.Height;
    winDesc.Format = ToDXGIFormat(mDesc.Format);
    winDesc.Stereo = FALSE;
    winDesc.SampleDesc.Count = 1;
    winDesc.SampleDesc.Quality = 0;
    winDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_BACK_BUFFER;
    winDesc.BufferCount = desc.BufferCount;
    winDesc.Scaling = DXGI_SCALING_STRETCH;
    /*
        about the 2 swap effect modes: blit(old) & flip
        https://devblogs.microsoft.com/directx/dxgi-flip-model/
        prio win7, blit into surface owned by window
        D3D9 FLIPEX share surface with desktop compositor

        sequential: preserve backbuffer content
        discard: won't preserve backbuffer content

    */
    winDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    IDXGISwapChain1* swapchain = nullptr;
    CheckDX12(gDX12GlobalState.Singletons.DXGIFactory->CreateSwapChainForHwnd(
        gDX12GlobalState.Singletons.D3DQueues[(u32)GfxApiQueueType::GraphicsQueue],
        desc.WindowHandle.ToNativeHandle(),
        &winDesc,
        nullptr,
        nullptr,
        &swapchain));
    CheckDX12(swapchain->QueryInterface(IID_PPV_ARGS(&mDX12SwapChain)));
    swapchain->Release();

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
    ID3D12DescriptorHeap*       heap;
    gDX12GlobalState.DescriptorManager
        ->AllocRange(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false, kBackbufferCount, mDescAllocHandle, cpuHandle, gpuHandle, heap);
    mCPUDescStart = cpuHandle.ptr;
    SetupBackbufferTextures();
}
DX12SwapChain::~DX12SwapChain()
{
    CleanupBackbufferTextures();
    SafeRelease(mDX12SwapChain);
    gDX12GlobalState.DescriptorManager->FreeRange(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false, mDescAllocHandle);
}
const GfxApiSwapChainDesc& DX12SwapChain::GetDesc()
{
    return mDesc;
}
void DX12SwapChain::Present(bool waitVSync)
{
    DX12Texture*           backbuffer = mBackbuffers[mDX12SwapChain->GetCurrentBackBufferIndex()];
    D3D12_RESOURCE_BARRIER barrier;
    if (backbuffer->EmitBarrier(D3D12_RESOURCE_STATE_PRESENT, &barrier))
    {
        ID3D12GraphicsCommandList4* cmdList = SetupCommandList(GfxApiQueueType::GraphicsQueue);
        cmdList->ResourceBarrier(1, &barrier);
        CheckDX12(cmdList->Close());
        ID3D12CommandList* cmd = cmdList;
        gDX12GlobalState.Singletons.D3DQueues[(u32)GfxApiQueueType::GraphicsQueue]->ExecuteCommandLists(1, &cmd);
        gDX12GlobalState.CommandListCache[(u32)D3D12_COMMAND_LIST_TYPE_DIRECT].Free(cmdList);
    }
    CheckDX12(mDX12SwapChain->Present(waitVSync ? 1 : 0, 0));
}
void DX12SwapChain::Update(const GfxApiSwapChainDesc& desc)
{
    mDesc = desc;
    CleanupBackbufferTextures();
    SetupBackbufferTextures();
}

u32 DX12SwapChain::GetCurrentBackbufferIndex()
{
    return mDX12SwapChain->GetCurrentBackBufferIndex();
}

void DX12SwapChain::GetBackbufferTextures(GfxApiTextureRef backbuffers[], u32 count)
{
    for (u32 i = 0; i < count; ++i)
    {
        backbuffers[i] = mTextureRefs[i];
    }
}
void DX12SwapChain::CleanupBackbufferTextures()
{
    for (u32 iBuffer = 0; iBuffer < mDesc.BufferCount; ++iBuffer)
    {
        mBackbuffers[iBuffer]->~DX12Texture();
        gDX12GlobalState.DX12TexturePool.Free(mTextureRefs[iBuffer]);
        mBackbuffers[iBuffer] = nullptr;
    }
}
void DX12SwapChain::SetupBackbufferTextures()
{
    for (u32 iBuffer = 0; iBuffer < kBackbufferCount; ++iBuffer)
    {
        mBackbuffers[iBuffer] = {};
    }

    GfxApiTextureDesc texDesc;
    texDesc.Width = mDesc.Width;
    texDesc.Height = mDesc.Height;
    texDesc.AccessFlags = GfxApiAccessFlags::GPUPrivate;
    texDesc.Format = mDesc.Format;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(ToCPUDescriptorHandle(mCPUDescStart));
    u32 stride = gDX12GlobalState.DescriptorManager->GetHeapIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    for (u32 iBuffer = 0; iBuffer < mDesc.BufferCount; ++iBuffer)
    {
        ID3D12Resource* res;
        CheckDX12(mDX12SwapChain->GetBuffer(iBuffer, IID_PPV_ARGS(&res)));
        res->SetName(BackBufferNames[iBuffer]);

        D3D12_RENDER_TARGET_VIEW_DESC vDesc;
        ZeroFill(vDesc);
        vDesc.Format = ToDXGIFormat(mDesc.Format);
        vDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

        gDX12GlobalState.Singletons.D3DDevice->CreateRenderTargetView(res, &vDesc, rtvHandle);
        std::tie((GfxApiTextureRef::UnderlyingHandle&)mTextureRefs[iBuffer], mBackbuffers[iBuffer]) =
            gDX12GlobalState.DX12TexturePool.Alloc();
        new (mBackbuffers[iBuffer]) DX12Texture(texDesc, res, D3D12_RESOURCE_STATE_COMMON, rtvHandle.ptr, true);
        rtvHandle.Offset(stride);
    }
}
} // namespace Omni

#endif
