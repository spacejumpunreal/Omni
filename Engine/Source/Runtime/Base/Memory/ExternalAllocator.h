#pragma once
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include <type_traits>

namespace Omni
{

using ExternalAllocationHandle = void*;
using ExtenralAllocationBlockId = void*;
struct ExternalAllocation
{
    u64                       Start;
    u64                       Size;
    ExternalAllocationHandle  Handle;
    ExtenralAllocationBlockId BlockId;
};

struct IExternalMemProvider
{
    virtual void Map(u64 size, u64 align, ExtenralAllocationBlockId& outBlockId) = 0;
    virtual void Unmap(ExtenralAllocationBlockId blockId) = 0;
    virtual u64  SuggestNewBlockSize(u64 reqSize) = 0;
};

template <typename T>
concept IsIExternalMemProvider = std::is_base_of_v<IExternalMemProvider, T>;

template <IsIExternalMemProvider Provider>
struct BestFitAllocator
{
public:
    BestFitAllocator();
    ~BestFitAllocator();
    ExternalAllocation Alloc(u64 size, u64 align);
    void               Free(ExternalAllocationHandle handle);
    void               Cleanup();

public:
    Provider mProvider;

public:
    using MemNodePtr = i32;
    using TFreeMap = PMRMultiMap<u64, MemNodePtr>;

protected:
    struct MemNode
    {
        MemNodePtr         Prev;
        MemNodePtr         Next;
        u64                Start;
        u64                Size : 63;
        u64                InUse : 1;
        void*              BlockId;
        TFreeMap::iterator MapItr;
    };

protected:
    FORCEINLINE MemNodePtr AllocNode();
    FORCEINLINE void       FreeNode(MemNodePtr nodePtr);

protected:
    TFreeMap           mFreeMap; // size to nodePtr map
    PMRVector<MemNode> mNodes;
    MemNodePtr         mFreeNodeList;
};

} // namespace Omni