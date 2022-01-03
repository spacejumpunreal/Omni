#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12SwapChain.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12Fence.h"
#include "Runtime/Core/GfxApi/DX12/DX12Texture.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/DX12/d3dx12.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineUtils.h"
#include "Runtime/Core/GfxApi/DX12/DX12CommandUtils.h"

namespace Omni
{
	//constants
	const wchar_t* BackBufferNames[] = {
		L"BackBuffer0",
		L"BackBuffer1",
		L"BackBuffer2",
	};

	DX12SwapChain::DX12SwapChain(const GfxApiSwapChainDesc& desc)
		: mDesc(desc)
		, mDX12SwapChain(nullptr)
	{

		DXGI_SWAP_CHAIN_DESC1 winDesc = {};
		winDesc.Width = mDesc.Width;
		winDesc.Height = mDesc.Height;
		//winDesc.Format = mDesc.Format;
		winDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
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
		CheckDX12(gDX12GlobalState.DXGIFactory->CreateSwapChainForHwnd(
			gDX12GlobalState.D3DGraphicsCommandQueue,
			desc.WindowHandle.ToNativeHandle(),
			&winDesc,
			nullptr,
			nullptr,
			&swapchain));
		CheckDX12(swapchain->QueryInterface(IID_PPV_ARGS(&mDX12SwapChain)));
		swapchain->Release();
		GfxApiTextureDesc texDesc;
		texDesc.Width = desc.Width;
		texDesc.Height = desc.Height;
		texDesc.AccessFlags = GfxApiAccessFlags::GPUWrite;
		texDesc.Format = desc.Format;
        D3D12_DESCRIPTOR_HEAP_TYPE heapType = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        {
            D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
            heapDesc.Type = heapType;
            heapDesc.NumDescriptors = desc.BufferCount;
            heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            heapDesc.NodeMask = 0;
            CheckDX12(gDX12GlobalState.D3DDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mTmpDescriptorHeap)));
        }

        for (u32 iBuffer = 0; iBuffer < MaxBackbuffers; ++iBuffer)
        {
            mBackbuffers[iBuffer] = {};
        }
        
        u32 stride = gDX12GlobalState.D3DDevice->GetDescriptorHandleIncrementSize(heapType);
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(mTmpDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
		for (u32 iBuffer = 0; iBuffer < desc.BufferCount; ++iBuffer)
		{
			ID3D12Resource* res;
			CheckDX12(mDX12SwapChain->GetBuffer(iBuffer, IID_PPV_ARGS(&res)));
			res->SetName(BackBufferNames[iBuffer]);
            gDX12GlobalState.D3DDevice->CreateRenderTargetView(res, nullptr, rtvHandle);
			mBackbuffers[iBuffer] = new DX12Texture(texDesc, res, D3D12_RESOURCE_STATE_COMMON, rtvHandle.ptr, true);
            rtvHandle.Offset(stride);
		}
	}
	DX12SwapChain::~DX12SwapChain()
	{
        auto fence = CreateFence(0, gDX12GlobalState.D3DDevice);
        UpdateFenceOnGPU(fence, 1, gDX12GlobalState.D3DGraphicsCommandQueue);
        WaitForFence(fence, 1);
        ReleaseFence(fence);

		for (u32 iBuffer = 0; iBuffer < mDesc.BufferCount; ++iBuffer)
		{
            delete mBackbuffers[iBuffer];
            mBackbuffers[iBuffer] = nullptr;
		}
        SafeRelease(mDX12SwapChain);
        SafeRelease(mTmpDescriptorHeap);
	}
	const GfxApiSwapChainDesc& DX12SwapChain::GetDesc()
	{
		return mDesc;
	}
	void DX12SwapChain::Present(bool waitVSync)
	{
        DX12Texture* backbuffer = mBackbuffers[mDX12SwapChain->GetCurrentBackBufferIndex()];
        D3D12_RESOURCE_BARRIER barrier;
        if (backbuffer->EmitBarrier(D3D12_RESOURCE_STATE_PRESENT, &barrier))
        {
            ID3D12GraphicsCommandList4* cmdList = SetupDirectCommandList();
            cmdList->ResourceBarrier(1, &barrier);
            CheckDX12(cmdList->Close());
            ID3D12CommandList* cmd = cmdList;
            gDX12GlobalState.D3DGraphicsCommandQueue->ExecuteCommandLists(1, &cmd);
            gDX12GlobalState.DirectCommandListCache.Free(cmdList);
        }
		CheckDX12(mDX12SwapChain->Present(waitVSync ? 1 : 0, 0));
	}
	void DX12SwapChain::Update(const GfxApiSwapChainDesc& desc)
	{
		(void)desc;
		NotImplemented("DX12SwapChain::Update");
	}

	u32 DX12SwapChain::GetCurrentBackbufferIndex()
	{
		return mDX12SwapChain->GetCurrentBackBufferIndex();
	}

	void DX12SwapChain::GetBackbufferTextures(GfxApiTextureRef backbuffers[], u32 count)
	{
        for (u32 i = 0; i < count; ++i)
        {
            backbuffers[i] = mBackbuffers[i];
        }
	}
}



#endif
