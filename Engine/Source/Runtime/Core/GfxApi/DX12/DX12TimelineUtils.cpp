#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12TimelineUtils.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include <d3d12.h>


namespace Omni
{

    void TimelineHelpers::RecycleDirectCommandAllocator(ID3D12CommandAllocator* commandAllocator)
    {
        gDX12GlobalState.CommandAllocatorCache[(u32)D3D12_COMMAND_LIST_TYPE_DIRECT].Free(commandAllocator);
    }
    void TimelineHelpers::ReleaseD3DObject(IUnknown* obj)
    {
        obj->Release();
    }
    void TimelineHelpers::ReleaseSharedObject(SharedObject* obj)
    {
        obj->Release();
    }
}

#endif
