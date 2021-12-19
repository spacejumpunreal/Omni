#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiConstants.h"
#include "Runtime/Core/GfxApi/GfxApiObject.h"
#include <d3d12.h>


namespace Omni
{
	class DX12SwapChain : public GfxApiSwapChain
	{
	public:
		static const u32 MaxBackbuffers = 3;
	public:
		DX12SwapChain(const GfxApiSwapChainDesc& desc);
		~DX12SwapChain();
		void Destroy() override;
		void Present(bool waitForVSync) override;
		SharedPtr<GfxApiTexture> GetCurrentBackbuffer() override;
	private:
		GfxApiSwapChainDesc mDesc;
		IDXGISwapChain3* mDX12SwapChain;
		ID3D12Resource* mBackbuffers[FrameResourceCount];
	};
}

#endif//OMNI_WINDOWS