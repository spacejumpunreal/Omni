#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12SwapChain.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/GfxApi/DX12/DX12Context.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/DX12/DX12Fence.h"


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
		CheckGfxApi(gDX12Context.DXGIFactory->CreateSwapChainForHwnd(
			gDX12Context.D3DGraphicsCommandQueue,
			desc.WindowHandle.ToNativeHandle(),
			&winDesc,
			nullptr,
			nullptr,
			&swapchain));
		CheckGfxApi(swapchain->QueryInterface(IID_PPV_ARGS(&mDX12SwapChain)));
		for (u32 iBuffer = 0; iBuffer < desc.BufferCount; ++iBuffer)
		{
			CheckGfxApi(mDX12SwapChain->GetBuffer(iBuffer, IID_PPV_ARGS(&mBackbuffers[iBuffer])));
			mBackbuffers[iBuffer]->SetName(BackBufferNames[iBuffer]);
		}
		
	}
	void DX12SwapChain::Destroy()
	{
		auto self = this;
		OMNI_DELETE(self, MemoryKind::GfxApi);
	}
	DX12SwapChain::~DX12SwapChain()
	{
		for (u32 iBuffer = 0; iBuffer < mDesc.BufferCount; ++iBuffer)
		{
			SafeRelease(mBackbuffers[iBuffer]);
		}
		gDX12Context.WaitGPUIdle();
		mDX12SwapChain->Release();
		mDX12SwapChain = nullptr;
	}
	void DX12SwapChain::Present(bool waitForVSync)
	{
		//printf("Buffer%d to be presented\n", mDX12SwapChain->GetCurrentBackBufferIndex());
		CheckGfxApi(mDX12SwapChain->Present(waitForVSync ? 1 : 0, 0));
		
	}
	SharedPtr<GfxApiTexture> DX12SwapChain::GetCurrentBackbuffer()
	{
		return SharedPtr<GfxApiTexture>();
	}
}



#endif