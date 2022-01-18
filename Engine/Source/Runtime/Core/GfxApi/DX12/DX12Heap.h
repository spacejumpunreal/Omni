#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiConstants.h"
#include "Runtime/Core/GfxApi/GfxApiNewDelete.h"
#include "Runtime/Core/GfxApi/GfxApiObject.h"

struct ID3D12Heap;
struct D3D12_HEAP_DESC;
enum D3D12_HEAP_TYPE;

namespace Omni
{

struct HeapAllocation
{
    ID3D12Heap* Heap;
    u64         Offset;
};

void        ToD3D12HeapType(D3D12_HEAP_TYPE& heapType, GfxApiAccessFlags accessFlag);
void        BuildHeapDesc(D3D12_HEAP_DESC& heapDesc, GfxApiAccessFlags heapUsage);
ID3D12Heap* CreateBufferHeap(u64 size, GfxApiAccessFlags heapUsage);
} // namespace Omni

#endif // OMNI_WINDOWS
