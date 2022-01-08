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

    constexpr IndexHandle NullIndexHandle = IndexHandle{ (u32)-1, (u32)-1 };

    template<typename TObject>
    struct RawPtrHandle
    {
        TObject*        Ptr;
    };
}
