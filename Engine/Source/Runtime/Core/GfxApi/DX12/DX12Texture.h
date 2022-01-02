#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiObject.h"
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"
#include "Runtime/Core/GfxApi/DX12/DX12Basics.h"


//forward decl
struct ID3D12Resource;


namespace Omni
{
	class DX12Texture final : public GfxApiTexture
	{
	public:
        DEFINE_GFX_API_OBJECT_NEW_DELETE();
		DX12Texture(const GfxApiTextureDesc& desc);
		DX12Texture(const GfxApiTextureDesc& desc, ID3D12Resource* res, DX12Descriptor descriptor, bool isOwner); //create from existing texture
		~DX12Texture();
		const GfxApiTextureDesc& GetDesc() override;

        DX12Descriptor GetCPUDescriptor();

	private:
		GfxApiTextureDesc	mDesc;
		ID3D12Resource*		mTexture;
        DX12Descriptor      mTmpCPUDescriptor;
        bool                mIsOwner;
	};
}

#endif//OMNI_WINDOWS
