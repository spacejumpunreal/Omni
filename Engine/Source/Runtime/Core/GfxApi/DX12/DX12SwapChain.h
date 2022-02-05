#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Memory/ExternalAllocation.h"
#include "Runtime/Core/GfxApi/GfxApiConstants.h"
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"
#include "Runtime/Core/GfxApi/GfxApiObject.h"
#include "Runtime/Core/GfxApi/DX12/DX12Basics.h"

//forward decl
struct IDXGISwapChain3;
struct ID3D12DescriptorHeap;

namespace Omni
{
    //forward decl
    class DX12Texture;

	class DX12SwapChain
	{
	public:
		DX12SwapChain(const GfxApiSwapChainDesc& desc);
		~DX12SwapChain();
		const GfxApiSwapChainDesc& GetDesc();
		void Present(bool waitVSync);
		void Update(const GfxApiSwapChainDesc& desc);
		u32 GetCurrentBackbufferIndex();
		void GetBackbufferTextures(GfxApiTextureRef backbuffers[], u32 count);
    protected:
        void CleanupBackbufferTextures();
        void SetupBackbufferTextures();
	private:
		GfxApiSwapChainDesc         mDesc;
		IDXGISwapChain3*            mDX12SwapChain;
        ExternalAllocationHandle    mDescAllocHandle;
        DX12Descriptor              mCPUDescStart;
        DX12Texture*                mBackbuffers[kBackbufferCount];
        GfxApiTextureRef            mTextureRefs[kBackbufferCount];
	};
}

#endif//OMNI_WINDOWS
