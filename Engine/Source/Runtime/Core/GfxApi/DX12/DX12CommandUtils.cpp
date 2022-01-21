#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/DX12/DX12CommandUtils.h"
#include "Runtime/Core/GfxApi/DX12/DX12GlobalState.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineUtils.h"
#include "Runtime/Core/GfxApi/DX12/DX12Utils.h"

namespace Omni
{

ID3D12GraphicsCommandList4* SetupCommandList(GfxApiQueueType queueType)
{
    ID3D12CommandAllocator*     allocator = gDX12GlobalState.CommandAllocatorCache[(u32)queueType].Alloc();
    ID3D12GraphicsCommandList4* cmdList = gDX12GlobalState.CommandListCache[(u32)queueType].Alloc();
    cmdList->Reset(allocator, nullptr);

    DX12TimelineManager::DX12BatchCB batchCB;
    switch (queueType)
    {
    case GfxApiQueueType::GraphicsQueue:
        batchCB = TimelineHelpers::CreateBatchCB(TimelineHelpers::RecycleDirectCommandAllocator, allocator);
        break;
    case GfxApiQueueType::CopyQueue:
        batchCB = TimelineHelpers::CreateBatchCB(TimelineHelpers::RecycleCopyCommandAllocator, allocator);
        break;
    default:
        NotImplemented();
        break;
    }
    gDX12GlobalState.TimelineManager->AddBatchCallback(queueType, batchCB);
    return cmdList;
}

} // namespace Omni

#endif // OMNI_WINDOWS
