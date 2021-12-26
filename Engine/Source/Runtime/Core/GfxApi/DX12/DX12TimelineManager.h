#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Misc/FunctorUtils.h"
#include "Runtime/Base/Misc/PrivateData.h"
#include "Runtime/Core/GfxApi/GfxApiDefs.h"

#if OMNI_WINDOWS

namespace Omni
{
    //forward decl

    //declaration
    class DX12ObjectLifeTimeManager
    {
    public:
        using DX12RecycleCB = Action1<void, void*>;
    public:
        DX12ObjectLifeTimeManager();
        ~DX12ObjectLifeTimeManager();
        void OnEvent(u64 eventSeq, GfxApiQueueType queueType);
        void AddEvent(GfxApiQueueType queueType, DX12RecycleCB action);
    private:
        PrivateData<1024, 16> mData;
    };
}


#endif//OMNI_WINDOWS
