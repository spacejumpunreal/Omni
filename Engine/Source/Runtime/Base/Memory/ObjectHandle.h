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
    THandleIndex Index;
    THandleGen   Gen;

public:
    static FORCEINLINE IndexHandle Null();
    friend FORCEINLINE bool        operator==(IndexHandle lhs, IndexHandle rhs)
    {
        return *reinterpret_cast<u64*>(&lhs) == *reinterpret_cast<u64*>(&rhs);
    }

    friend FORCEINLINE bool operator!=(IndexHandle lhs, IndexHandle rhs)
    {
        return *reinterpret_cast<u64*>(&lhs) != *reinterpret_cast<u64*>(&rhs);
    }
};

constexpr IndexHandle   NullIndexHandle = IndexHandle{(u32)0, (u32)0};
FORCEINLINE IndexHandle IndexHandle::Null()
{
    return NullIndexHandle;
}

struct RawPtrHandle
{
public:
    u64 Addr : 48;
    u64 Gen : 16;

public:
    static FORCEINLINE RawPtrHandle Null();
    friend FORCEINLINE bool         operator==(RawPtrHandle lhs, RawPtrHandle rhs)
    {
        return *reinterpret_cast<u64*>(&lhs) == *reinterpret_cast<u64*>(&rhs);
    }

    friend FORCEINLINE bool operator!=(RawPtrHandle lhs, RawPtrHandle rhs)
    {
        return *reinterpret_cast<u64*>(&lhs) != *reinterpret_cast<u64*>(&rhs);
    }
};

constexpr RawPtrHandle NullPtrHandle = RawPtrHandle{
    .Addr = 0,
    .Gen = 0,
};
static_assert(sizeof(RawPtrHandle) == sizeof(u64));

FORCEINLINE RawPtrHandle RawPtrHandle::Null()
{
    return NullPtrHandle;
}

} // namespace Omni
