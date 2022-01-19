#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Memory/ExternalAllocator.h"
#include "Runtime/Core/GfxApi/GfxApiConstants.h"
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"
#include "Runtime/Core/GfxApi/GfxApiObject.h"

struct ID3D12Resource;

namespace Omni
{

class DX12BufferManager
{
public:
    static DX12BufferManager* Create();
    void                      Destroy();

    void AllocBuffer(const GfxApiBufferDesc& desc, ID3D12Resource*& dx12Buffer, ExternalAllocationHandle& allocHandle);
    void FreeBuffer(GfxApiAccessFlags accessFlag, ID3D12Resource* dx12Buffer, ExternalAllocationHandle allocHandle);
};

} // namespace Omni

#endif // OMNI_WINDOWS
