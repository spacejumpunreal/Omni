#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12Heap.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/DX12/d3dx12.h"

namespace Omni
{

// https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_resource_desc
void ToD3D12HeapType(D3D12_HEAP_TYPE& heapType, GfxApiAccessFlags accessFlag)
{
    if (Any(accessFlag & GfxApiAccessFlags::CPURead) && None(accessFlag & GfxApiAccessFlags::CPUWrite))
        heapType = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_READBACK;
    else if (None(accessFlag & GfxApiAccessFlags::GPUWrite) && Any(accessFlag & GfxApiAccessFlags::CPUWrite))
        heapType = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_UPLOAD;
    else if (None(accessFlag & (GfxApiAccessFlags::CPURead | GfxApiAccessFlags::CPUWrite)))
        heapType = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;
    else
    {
        CheckAlways(false, "unknown GPU resource access pattern");
        heapType = D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT;
    }
}

void BuildHeapDesc(D3D12_HEAP_DESC& heapDesc, GfxApiAccessFlags heapUsage)
{
    heapDesc.Alignment = 64 * 1024; // only valid option
    heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
    D3D12_HEAP_TYPE heapType;
    ToD3D12HeapType(heapType, heapUsage);
    heapDesc.Properties = CD3DX12_HEAP_PROPERTIES(heapType);
    heapDesc.SizeInBytes = 0;
}

ID3D12Heap* CreateBufferHeap(u64 size, GfxApiAccessFlags heapUsage)
{
    ID3D12Heap*     heap;
    D3D12_HEAP_DESC heapDesc;
    BuildHeapDesc(heapDesc, heapUsage);
    heapDesc.SizeInBytes = size;
    CheckDX12(gDX12GlobalState.Singletons.D3DDevice->CreateHeap(&heapDesc, IID_PPV_ARGS(&heap)));
    return heap;
}
} // namespace Omni

#endif
