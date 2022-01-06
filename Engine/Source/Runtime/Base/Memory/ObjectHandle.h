#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"

namespace Omni
{
    using THandleGen = u32;
    using THandleIndex = u32;

    struct IndexHandle
    {
        THandleGen      Gen;
        THandleIndex    Index;
    };

    template<typename TObject>
    struct RawPtrHandle
    {
        TObject*        Ptr;
    };
}
