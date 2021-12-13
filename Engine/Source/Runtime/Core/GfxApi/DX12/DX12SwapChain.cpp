#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12SwapChain.h"

namespace Omni
{
	DX12SwapChain::DX12SwapChain(const GfxApiSwapChainDesc& desc)
		: mDesc(desc)
	{

	}
	void DX12SwapChain::Present()
	{
	}
	SharedPtr<GfxApiTexture> DX12SwapChain::GetCurrentBackbuffer()
	{
		return SharedPtr<GfxApiTexture>(nullptr);
	}
}



#endif