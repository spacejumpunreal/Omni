#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiConstants.h"
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"
#include "Runtime/Core/GfxApi/GfxApiObject.h"
#include "Runtime/Base/Memory/ExternalAllocation.h"
#include "Runtime/Core/GfxApi/DX12/DX12ForwardDecl.h"

namespace Omni
{
class DX12Buffer
{
public:
    DX12Buffer(const GfxApiBufferDesc& desc);
    ~DX12Buffer();
    const GfxApiBufferDesc GetDesc();

private:
    GfxApiBufferDesc         mDesc;
    ID3D12Resource*          mDX12Buffer;
    ExternalAllocationHandle mAllocHandle;
    D3D12_RESOURCE_STATES    mResourceState;
};
} // namespace Omni

#endif // OMNI_WINDOWS
