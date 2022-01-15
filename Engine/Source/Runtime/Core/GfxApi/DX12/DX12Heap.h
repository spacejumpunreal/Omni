#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiConstants.h"
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"
#include "Runtime/Core/GfxApi/GfxApiObject.h"

struct ID3D12Heap;

namespace Omni
{

struct HeapAllocation
{
    ID3D12Heap* Heap;
    u64         Offset;
};

ID3D12Heap* CreateBufferHeap(u64 size, GfxApiAccessFlags heapUsage);
} // namespace Omni

#endif // OMNI_WINDOWS
