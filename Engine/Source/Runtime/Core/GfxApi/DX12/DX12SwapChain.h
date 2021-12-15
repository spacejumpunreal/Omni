#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiObject.h"
#include <d3d12.h>


namespace Omni
{
	class DX12SwapChain : public GfxApiSwapChain
	{
	public:
		DX12SwapChain(const GfxApiSwapChainDesc& desc);
		void Destroy() override;
		~DX12SwapChain();
		void Present() override ;
		SharedPtr<GfxApiTexture> GetCurrentBackbuffer() override;
	private:
		GfxApiSwapChainDesc mDesc;
		IDXGISwapChain1* mDX12SwapChain;
	};
}

#endif//OMNI_WINDOWS