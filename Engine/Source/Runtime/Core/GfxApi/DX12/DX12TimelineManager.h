#pragma once
#include "Runtime/Prelude/Omni.h"

#if OMNI_WINDOWS
#include "Runtime/Base/Misc/FunctorUtils.h"
#include "Runtime/Base/Misc/PrivateData.h"
#include "Runtime/Core/GfxApi/GfxApiDefs.h"

struct ID3D12CommandList;
struct ID3D12Device;

namespace Omni
{
// forward decl
class DX12GpuEvent
{
};

// constants
constexpr u64 BatchIdResetValue = 0;

// declaration
class DX12TimelineManager
{ // batch = a gpu event on a specific queue, and some CPU callbacks that should run after the gpu event happened
public:
    using DX12BatchCB = Action1<void, void*>;
public:
    static DX12TimelineManager* Create(ID3D12Device* dev);
    void                        Destroy();

    bool CloseBatchAndSignalOnGPU(GfxApiQueueType queueType, ID3D12CommandQueue* queue, u64& batchId, bool force);
    void WaitBatchFinishOnGPU(GfxApiQueueType queueType, u64 batchId);
    bool IsBatchFinishedOnGPU(GfxApiQueueType queueType, u64 batchId);
    void PollBatch(GfxApiQueueMask queueMask);
    u64  AddBatchCallback(GfxApiQueueType queueType, DX12BatchCB action);
    void AddMultiQueueBatchCallback(GfxApiQueueType queues[], u32 queueCount, DX12BatchCB action);
};
} // namespace Omni

#endif // OMNI_WINDOWS
