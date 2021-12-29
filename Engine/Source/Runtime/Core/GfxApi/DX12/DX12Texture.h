#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiObject.h"
#include "Runtime/Core/GfxApi/DX12/DX12Basics.h"

//forward decl
struct ID3D12Resource;

namespace Omni
{
	class DX12Texture : public GfxApiTexture
	{
	public:
		DX12Texture(const GfxApiTextureDesc& desc);
		DX12Texture(const GfxApiTextureDesc& desc, ID3D12Resource* res); //create from existing texture
		~DX12Texture();
		void Destroy() override;
		const GfxApiTextureDesc& GetDesc() override;

        DX12Descriptor GetCPUDescriptor();

	private:
		GfxApiTextureDesc	mDesc;
		ID3D12Resource*		mTexture;
	};
}


#endif
