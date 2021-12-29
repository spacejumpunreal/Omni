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
        gDX12GlobalState.DirectCommandAllocatorCache.Free(commandAllocator);
    }
    void TimelineHelpers::ReleaseObject(IUnknown* obj)
    {
        obj->Release();
    }

}

#endif
