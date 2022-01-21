#pragma once
#include "Runtime/Prelude/Omni.h"
#if OMNI_WINDOWS
#include "Runtime/Core/GfxApi/GfxApiObject.h"

namespace Omni
{
class DX12GpuEvent
{
public:
    DX12GpuEvent(u64 batchId, GfxApiQueueType queueType) : BatchId(batchId), QueueType(queueType)
    {
    }
    ~DX12GpuEvent()
    {
    }

public:
    const u64             BatchId;
    const GfxApiQueueType QueueType;
};
} // namespace Omni

#endif // OMNI_WINDOWS
