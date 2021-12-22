#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12Texture.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Platform/OSUtils_Windows.h"


namespace Omni
{
	DX12Texture::DX12Texture(const GfxApiTextureDesc& desc)
		: mDesc(desc)
		, mTexture(nullptr)
	{
		NotImplemented();
	}
	DX12Texture::DX12Texture(const GfxApiTextureDesc& desc, ID3D12Resource* res)
		: mDesc(desc)
		, mTexture(res)
	{
	}
	DX12Texture::~DX12Texture()
	{
		CheckDebug(mTexture);
		SafeRelease(mTexture);
	}
	void DX12Texture::Destroy()
	{
		auto self = this;
		OMNI_DELETE(self, MemoryKind::GfxApi);
	}
	const GfxApiTextureDesc& DX12Texture::GetDesc()
	{
		return mDesc;
	}
}

#endif