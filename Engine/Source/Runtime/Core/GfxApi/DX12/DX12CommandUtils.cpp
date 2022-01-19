#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12CommandUtils.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineUtils.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"

namespace Omni
{
ID3D12GraphicsCommandList4* SetupDirectCommandList()
{
    ID3D12CommandAllocator*     allocator = gDX12GlobalState.DirectCommandAllocatorCache.Alloc();
    ID3D12GraphicsCommandList4* cmdList = gDX12GlobalState.DirectCommandListCache.Alloc();
    cmdList->Reset(allocator, nullptr);

    gDX12GlobalState.TimelineManager->AddBatchCallback(
        GfxApiQueueType::GraphicsQueue,
        TimelineHelpers::CreateBatchCB(TimelineHelpers::RecycleDirectCommandAllocator, allocator));
    return cmdList;
}

ID3D12GraphicsCommandList4* SetupCopyCommandList()
{
    return nullptr;
    #if 0
    ID3D12CommandAllocator*     allocator = gDX12GlobalState.DirectCommandAllocatorCache.Alloc();
    ID3D12GraphicsCommandList4* cmdList = gDX12GlobalState.DirectCommandListCache.Alloc();
    cmdList->Reset(allocator, nullptr);
    #endif
}

} // namespace Omni

#endif // OMNI_WINDOWS
