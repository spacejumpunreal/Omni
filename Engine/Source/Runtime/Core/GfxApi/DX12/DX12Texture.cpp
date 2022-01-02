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
        , mTmpCPUDescriptor(NullDX12Descriptor)
	{
		NotImplemented();
	}
    //this is for texture that was owned by others
	DX12Texture::DX12Texture(const GfxApiTextureDesc& desc, ID3D12Resource* res, DX12Descriptor descriptor, bool isOwner)
		: mDesc(desc)
		, mTexture(res)
        , mTmpCPUDescriptor(descriptor)
        , mIsOwner(isOwner)
	{
	}
	DX12Texture::~DX12Texture()
	{
        if (mIsOwner)
        {
            CheckDebug(mTexture);
            SafeRelease(mTexture);
        }
		
	}
	const GfxApiTextureDesc& DX12Texture::GetDesc()
	{
		return mDesc;
	}
    DX12Descriptor DX12Texture::GetCPUDescriptor()
    {
        return mTmpCPUDescriptor;
    }
}

#endif
