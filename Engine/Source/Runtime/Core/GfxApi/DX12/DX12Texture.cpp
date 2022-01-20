#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12Texture.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Platform/OSUtils_Windows.h"
#include "Runtime/Core/GfxApi/DX12/d3dx12.h"

namespace Omni
{
DX12Texture::DX12Texture(const GfxApiTextureDesc& desc)
    : DX12Resource(CalcInitialStateFromAccessFlag(desc.AccessFlags))
    , mDesc(desc)
    , mTmpCPUDescriptor(NullDX12Descriptor)
{
    NotImplemented();
}
// this is for texture that was owned by others
DX12Texture::DX12Texture(const GfxApiTextureDesc& desc,
                         ID3D12Resource*          res,
                         D3D12_RESOURCE_STATES    initState,
                         DX12Descriptor           descriptor,
                         bool                     isOwner)
    : DX12Resource(initState)
    , mDesc(desc)
    , mTmpCPUDescriptor(descriptor)
    , mIsOwner(isOwner)
{
    mDX12Resource = res;
}
DX12Texture::~DX12Texture()
{
    if (mIsOwner)
    {
        CheckDebug(mDX12Resource);
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

} // namespace Omni

#endif
