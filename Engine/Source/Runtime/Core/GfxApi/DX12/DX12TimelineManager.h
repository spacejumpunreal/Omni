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
    //forward decl

    //constants
    constexpr u64 BatchIdResetValue = 0;

    //declaration
    class DX12TimelineManager
    {
    public:
        using DX12RecycleCB = Action1<void, void*>;
    public:
        DX12TimelineManager(ID3D12Device* dev);
        ~DX12TimelineManager();
        u64 CloseBatchAndSignalOnGPU(GfxApiQueueType queueType, ID3D12CommandQueue* queue);
        void WaitBatchFinishOnGPU(GfxApiQueueType queueType, u64 batchId);
        bool IsBatchFinishedOnGPU(GfxApiQueueType queueType, u64 batchId);
        u64 AddBatchEvent(GfxApiQueueType queueType, DX12RecycleCB action);
    private:
        PrivateData<1024, 16> mData;
    };
}


#endif//OMNI_WINDOWS
