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
    gDX12GlobalState.CommandAllocatorCache[(u32)GfxApiQueueType::GraphicsQueue].Free(commandAllocator);
}
void TimelineHelpers::RecycleComputeCommandAllocator(ID3D12CommandAllocator* commandAllocator)
{
    gDX12GlobalState.CommandAllocatorCache[(u32)GfxApiQueueType::ComputeQueue].Free(commandAllocator);
}
void TimelineHelpers::RecycleCopyCommandAllocator(ID3D12CommandAllocator* commandAllocator)
{
    gDX12GlobalState.CommandAllocatorCache[(u32)GfxApiQueueType::CopyQueue].Free(commandAllocator);
}
void TimelineHelpers::ReleaseD3DObject(IUnknown* obj)
{
    obj->Release();
}
void TimelineHelpers::ReleaseSharedObject(SharedObject* obj)
{
    obj->Release();
}
} // namespace Omni

#endif
