#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiObject.h"
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"
#include "Runtime/Core/GfxApi/DX12/DX12Basics.h"
#include "Runtime/Core/GfxApi/DX12/d3dx12.h"

//forward decl
struct ID3D12Resource;
struct ID3D12CommandList;
struct D3D12_RESOURCE_BARRIER;


namespace Omni
{
	class DX12Texture
	{
	public:
		DX12Texture(const GfxApiTextureDesc& desc);
		DX12Texture(const GfxApiTextureDesc& desc, ID3D12Resource* res, D3D12_RESOURCE_STATES initState,
                    DX12Descriptor descriptor, bool isOwner); //create from existing texture
		~DX12Texture();
		const GfxApiTextureDesc& GetDesc();
        ID3D12Resource* GetDX12Texture() { return mTexture; }
        DX12Descriptor GetCPUDescriptor();
        bool EmitBarrier(D3D12_RESOURCE_STATES newState, D3D12_RESOURCE_BARRIER* barrier);

	private:
		GfxApiTextureDesc	        mDesc;
		ID3D12Resource*		        mTexture;
        DX12Descriptor              mTmpCPUDescriptor;
        D3D12_RESOURCE_STATES       mResourceState;
        bool                        mIsOwner;
	};
}

#endif//OMNI_WINDOWS
