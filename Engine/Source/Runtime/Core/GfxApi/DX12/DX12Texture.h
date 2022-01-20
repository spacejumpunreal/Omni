#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiObject.h"
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"
#include "Runtime/Core/GfxApi/DX12/DX12Basics.h"
#include "Runtime/Core/GfxApi/DX12/DX12ForwardDecl.h"
#include "Runtime/Core/GfxApi/DX12/DX12Resource.h"

namespace Omni
{
class DX12Texture : public DX12Resource
{
public:
    DX12Texture(const GfxApiTextureDesc& desc);
    DX12Texture(const GfxApiTextureDesc& desc,
                ID3D12Resource*          res,
                D3D12_RESOURCE_STATES    initState,
                DX12Descriptor           descriptor,
                bool                     isOwner); // create from existing texture
    ~DX12Texture();
    const GfxApiTextureDesc& GetDesc();
    DX12Descriptor GetCPUDescriptor();
private:
    GfxApiTextureDesc     mDesc;
    DX12Descriptor        mTmpCPUDescriptor;
    bool                  mIsOwner;
};
} // namespace Omni

#endif // OMNI_WINDOWS
