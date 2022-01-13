#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Memory/HandleObjectPoolImpl.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"
#include "Runtime/Core/GfxApi/DX12/d3dx12.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineUtils.h"
#include "Runtime/Core/GfxApi/DX12/d3dx12.h"


namespace Omni
{
    //https://docs.microsoft.com/en-us/windows/win32/api/d3d12/ns-d3d12-d3d12_resource_desc
    ID3D12Heap* CreateBufferHeap(u64 size, BufferHeapUsageFlag heapUsage)
    {
        D3D12_HEAP_DESC heapDesc;
        heapDesc.Alignment = 64 * 1024;//only valid option
        heapDesc.Flags = D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
        heapDesc.Properties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE::D3D12_HEAP_TYPE_DEFAULT)
        heapDesc.SizeInBytes = size;
        gDX12GlobalState.Singletons.D3DDevice->CreateHeap()
    }
}



#endif
