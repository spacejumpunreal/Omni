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
        THandleIndex    Index;
        THandleGen      Gen;
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

    struct RawPtrHandle
    {
    public:
        union
        {
            struct
            {
                u64  Addr: 48;
                u16  Gen: 16;
            } AddrGen;
            u8* Ptr;
        };
        
    public:
        friend FORCEINLINE bool operator==(RawPtrHandle lhs, RawPtrHandle rhs)
        {
            return lhs.Ptr == rhs.Ptr;
        }

        friend FORCEINLINE bool operator!=(RawPtrHandle lhs, RawPtrHandle rhs)
        {
            return lhs.Ptr != rhs.Ptr;
        }
    };

    constexpr RawPtrHandle NullPtrHandle = RawPtrHandle{ 
        .AddrGen = {
            .Addr = (1ull << 48) - 1,
            .Gen = 0xffff,
        }
    };
}
