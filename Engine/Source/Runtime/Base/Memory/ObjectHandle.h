#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"

namespace Omni
{
    using THandleGen = u32;
    using THandleIndex = u32;

    struct IndexHandle
    {
    public:
        THandleGen      Gen;
        THandleIndex    Index;
    public:
        friend FORCEINLINE bool operator==(IndexHandle lhs, IndexHandle rhs) 
        {
            return *reinterpret_cast<u64*>(&lhs) == *reinterpret_cast<u64*>(&rhs);
        }

        friend FORCEINLINE bool operator!=(IndexHandle lhs, IndexHandle rhs)
        {
            return *reinterpret_cast<u64*>(&lhs) != *reinterpret_cast<u64*>(&rhs);
        }
    };

    constexpr IndexHandle NullIndexHandle = IndexHandle{ (u32)-1, (u32)-1 };

    template<typename TObject>
    struct RawPtrHandle
    {
        TObject*        Ptr;
    };
}
