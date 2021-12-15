#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiObject.h"
#include <d3d12.h>


namespace Omni
{
	class DX12SwapChain : public GfxApiSwapChain
	{
	public:
		DX12SwapChain(const GfxApiSwapChainDesc& desc);
		void Present();
		SharedPtr<GfxApiTexture> GetCurrentBackbuffer();
	private:
		GfxApiSwapChainDesc mDesc;
		IDXGISwapChain1* mDX12SwapChain;
	};
}

#endif//OMNI_WINDOWS