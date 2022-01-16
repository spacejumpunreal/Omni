#pragma once
#include "Runtime/Base/Memory/ExternalAllocator.h"
#include "Runtime/Base/Misc/ArrayUtils.h"

namespace Omni
{

constexpr i32 NullMemNodePtr = -1;

template <IsIExternalMemProvider Provider>
BestFitAllocator<Provider>::BestFitAllocator() : mFreeNodeList(NullMemNodePtr)
{
}

template <IsIExternalMemProvider Provider>
BestFitAllocator<Provider>::~BestFitAllocator()
{
    Cleanup();
}

template <IsIExternalMemProvider Provider>
ExternalAllocation BestFitAllocator<Provider>::Alloc(u64 size, u64 align)
{
    auto       it = mFreeMap.lower_bound(size);
    MemNodePtr availNodePtr = NullMemNodePtr;
    while (it != mFreeMap.end())
    { // try find an avail node
        MemNode& node = mNodes[it->second];
        i64      remain = (node.Start + node.Size) - (AlignUpSize(node.Start, align) + size);
        if (remain > 0)
        {
            availNodePtr = it->second;
            mFreeMap.erase(it);
            break;
        }
        ++it;
    }
    if (availNodePtr == NullMemNodePtr)
    { // create new node if no avail
        u64 blockSize = mProvider.SuggestNewBlockSize(size);
        availNodePtr = AllocNode();
        MemNode&   newNode = mNodes[availNodePtr];
        newNode.Prev = newNode.Next = NullMemNodePtr;
        newNode.Start = 0;
        newNode.Size = blockSize;
        newNode.InUse = 0;
        mProvider.Map(blockSize, align, newNode.BlockId);
    }
    {
        MemNode& availNode0 = mNodes[availNodePtr];
        u64      usedStart = AlignUpSize(availNode0.Start, align);
        u64      usedEnd = usedStart + size;
        u64      lRemainSize = usedStart - availNode0.Start;
        u64      rRemainSize = availNode0.Start + availNode0.Size - usedEnd;
        if (lRemainSize > 0) 
        {
            MemNodePtr lRemainPtr = AllocNode();
            MemNode&   availNode1 = mNodes[availNodePtr];
            MemNode&   lRemain = mNodes[lRemainPtr];
            lRemain.Prev = availNode1.Prev;
            lRemain.Next = availNodePtr;
            lRemain.Start = availNode1.Start;
            lRemain.Size = lRemainSize;
            lRemain.InUse = 0;
            lRemain.BlockId = availNode1.BlockId;
            lRemain.MapItr = mFreeMap.emplace(std::make_pair(lRemainSize, lRemainPtr));

            if (availNode1.Prev != NullMemNodePtr)
            {
                mNodes[availNode1.Prev].Next = lRemainPtr;
            }
            availNode1.Prev = lRemainPtr;
            availNode1.Start = usedStart;
        }
        if (rRemainSize > 0)
        {
            MemNodePtr rRemainPtr = AllocNode();
            MemNode&   availNode2 = mNodes[availNodePtr];
            MemNode&   rRemain = mNodes[rRemainPtr];
            rRemain.Prev = availNodePtr;
            rRemain.Next = availNode2.Next;
            rRemain.Start = usedEnd;
            rRemain.Size = rRemainSize;
            rRemain.InUse = 0;
            rRemain.BlockId = availNode2.BlockId;
            rRemain.MapItr = mFreeMap.emplace(std::make_pair(rRemainSize, rRemainPtr));

            if (availNode2.Next != NullMemNodePtr)
            {
                mNodes[availNode2.Next].Prev = rRemainPtr;
            }
            availNode2.Next = rRemainPtr;
        }
        MemNode& availNode3 = mNodes[availNodePtr];
        availNode3.Size = usedEnd - usedStart;
        availNode3.InUse = 1;

        return ExternalAllocation {
            .Start = usedStart,
            .Size = availNode3.Size,
            .Handle = (void*)(u64)availNodePtr,
            .BlockId = availNode3.BlockId,
        };
    }
}

template <IsIExternalMemProvider Provider>
void BestFitAllocator<Provider>::Free(ExternalAllocationHandle handle)
{
    MemNodePtr theNodePtr = (MemNodePtr)(u64)handle;
    MemNode& theNode = mNodes[theNodePtr];
    CheckDebug(theNode.InUse == 1);
    theNode.InUse = 0;
    if (theNode.Prev != NullMemNodePtr)
    {//handle merge
        MemNode& lNode = mNodes[theNode.Prev];
        if (lNode.InUse == 0 && lNode.Start + lNode.Size == theNode.Start && lNode.BlockId == theNode.BlockId)
        {
            if (lNode.Prev != NullMemNodePtr)
            {
                mNodes[lNode.Prev].Next = theNodePtr;
            }
            mFreeMap.erase(lNode.MapItr);
            theNode.Prev = lNode.Prev;
            theNode.Start = lNode.Start;
            theNode.Size += lNode.Size;
        }
    }
    if (theNode.Next != NullMemNodePtr)
    {//handle merge
        MemNode& rNode = mNodes[theNode.Next];
        if (rNode.InUse == 0 && rNode.Start == theNode.Start + theNode.Size && rNode.BlockId == theNode.BlockId)
        {
            if (rNode.Next != NullMemNodePtr)
            {
                mNodes[rNode.Next].Prev = theNodePtr;
            }
            mFreeMap.erase(rNode.MapItr);
            theNode.Next = rNode.Next;
            theNode.Size += rNode.Size;
        }
    }
    theNode.MapItr = mFreeMap.insert({theNode.Size, theNodePtr});
}

template <IsIExternalMemProvider Provider>
void BestFitAllocator<Provider>::Cleanup()
{
    for (auto sizeAndNodePtr : mFreeMap)
    {
        mProvider.Unmap(mNodes[sizeAndNodePtr.second].BlockId);
    }
    mFreeMap.clear();
    mNodes.clear();
    mFreeNodeList = NullMemNodePtr;
}

template <IsIExternalMemProvider Provider>
BestFitAllocator<Provider>::MemNodePtr BestFitAllocator<Provider>::AllocNode()
{
    if (mFreeNodeList == NullMemNodePtr)
    {
        mNodes.emplace_back();
        return (MemNodePtr)mNodes.size() - 1;
    }
    MemNodePtr ret = mFreeNodeList;
    mFreeNodeList = mNodes[ret].Next;
    return ret;
}

template <IsIExternalMemProvider Provider>
void BestFitAllocator<Provider>::FreeNode(BestFitAllocator<Provider>::MemNodePtr nodePtr)
{
    mNodes[nodePtr].Next = mFreeNodeList;
    mFreeNodeList = nodePtr;
}

} // namespace Omni
