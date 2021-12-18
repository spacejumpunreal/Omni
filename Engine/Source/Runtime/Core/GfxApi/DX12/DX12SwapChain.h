#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiObject.h"
#include <d3d12.h>


namespace Omni
{
	class DX12SwapChain : public GfxApiSwapChain
	{
	public:
		DX12SwapChain(const GfxApiSwapChainDesc& desc);
		~DX12SwapChain();
		void Destroy() override;
		void Present(bool waitForVSync) override;
		SharedPtr<GfxApiTexture> GetCurrentBackbuffer() override;
	private:
		GfxApiSwapChainDesc mDesc;
		IDXGISwapChain1* mDX12SwapChain;
	};
}

#endif//OMNI_WINDOWS