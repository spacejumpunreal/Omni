#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiConstants.h"
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"
#include "Runtime/Core/GfxApi/GfxApiObject.h"

//forward decl
struct IDXGISwapChain3;
struct ID3D12DescriptorHeap;

namespace Omni
{
    //forward decl
    class DX12Texture;

	class DX12SwapChain final : public GfxApiSwapChain
	{
	public:
		static const u32 MaxBackbuffers = 3;
	public:
		DX12SwapChain(const GfxApiSwapChainDesc& desc);
		~DX12SwapChain();
		const GfxApiSwapChainDesc& GetDesc() override;
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
        ID3D12DescriptorHeap*       mTmpDescriptorHeap;
		DX12Texture*                mBackbuffers[MaxBackbuffers];
	};
}

#endif//OMNI_WINDOWS
