#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiConstants.h"
#include "Runtime/Core/GfxApi/GfxApiObject.h"

//forward decl
struct IDXGISwapChain3;

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
		const GfxApiSwapChainDesc& GetDesc();
		void Present(bool waitForVSync) override;
		void Update(const GfxApiSwapChainDesc& desc) override;
		u32 GetCurrentBackbufferIndex() override;
		GfxApiTextureRef GetCurrentBackbuffer() override;
	private:
		GfxApiSwapChainDesc mDesc;
		IDXGISwapChain3* mDX12SwapChain;
		GfxApiTextureRef mBackbuffers[MaxBackbuffers];
	};
}

#endif//OMNI_WINDOWS