#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiConstants.h"
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"
#include "Runtime/Core/GfxApi/GfxApiObject.h"
#include "Runtime/Base/Memory/ExternalAllocation.h"
#include "Runtime/Core/GfxApi/DX12/DX12ForwardDecl.h"
#include "Runtime/Core/GfxApi/DX12/DX12Resource.h"

namespace Omni
{
class DX12Buffer : public DX12Resource
{
public:
    DX12Buffer(const GfxApiBufferDesc& desc);
    ~DX12Buffer();
    const GfxApiBufferDesc GetDesc();
    void*                  Map(u32 beginOffset, u32 mapSize);
    void                   Unmap(u32 beginOffset, u32 mapSize);

private:
    GfxApiBufferDesc         mDesc;
    ExternalAllocationHandle mAllocHandle;
};
} // namespace Omni

#endif // OMNI_WINDOWS
