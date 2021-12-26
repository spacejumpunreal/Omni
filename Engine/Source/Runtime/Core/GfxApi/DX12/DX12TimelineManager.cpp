#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/GfxApi/DX12/DX12TimelineManager.h"
#include "Runtime/Base/Container/PMRContainers.h"

#if OMNI_WINDOWS

namespace Omni
{
    struct LifeTimeBatch
    {
        PMRVector<DX12ObjectLifeTimeManager::DX12RecycleCB> Callbacks;
    };

    struct DX12ObjectLifeTimeManagerPrivateData
    {
    public:
        DX12ObjectLifeTimeManagerPrivateData()
        {}
    public:
    };

    DX12ObjectLifeTimeManager::DX12ObjectLifeTimeManager()
        : mData(PrivateDataType<DX12ObjectLifeTimeManagerPrivateData>{})
    {
        //auto& self = mData.Ref<DX12ObjectLifeTimeManagerPrivateData>();
    }
    DX12ObjectLifeTimeManager::~DX12ObjectLifeTimeManager()
    {
        mData.DestroyAs<DX12ObjectLifeTimeManagerPrivateData>();
    }
    void DX12ObjectLifeTimeManager::OnEvent(u64 eventSeq, GfxApiQueueType queueType)
    {
    }
    void DX12ObjectLifeTimeManager::AddEvent(GfxApiQueueType queueType, DX12RecycleCB action)
    {
    }
}


#endif//OMNI_WINDOWS
