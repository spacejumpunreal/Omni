#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12SwapChain.h"
#include "Runtime/Core/GfxApi/DX12/DX12Context.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"

namespace Omni
{
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

		CheckGfxApi(gDX12Context.DXGIFactory->CreateSwapChainForHwnd(
			gDX12Context.D3DCommandQueue,
			desc.WindowHandle.ToNativeHandle(),
			&winDesc,
			nullptr,
			nullptr,
			&mDX12SwapChain));
	}
	void DX12SwapChain::Present()
	{
	}
	SharedPtr<GfxApiTexture> DX12SwapChain::GetCurrentBackbuffer()
	{
		return SharedPtr<GfxApiTexture>();
	}
}



#endif