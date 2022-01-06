#include "Runtime/Base/BasePCH.h"
#include "Runtime/Base/Memory/HandleObjectPool.h"
#include "Runtime/Base/Misc/ArrayUtils.h"
#include "Runtime/Base/Misc/AssertUtils.h"

namespace Omni
{
    bool ObjectArrayPoolBase::IsValid(IndexHandle handle)
    {
        u32 pageIdx = handle.Index >> mPageObjCountPow;
        u32 inPageIdx = handle.Index & ((1u << mPageObjCountPow) - 1u);
        if (pageIdx >= mPageTable.size())
            return false;
        u8* page = mPageTable[pageIdx];
        u8* genPtr = page + mObjectSize * inPageIdx + mGenOffset;
        THandleGen objGen = *(THandleGen*)genPtr;
        return objGen == handle.Gen;
    }
    void ObjectArrayPoolBase::Finalize()
    {
        (&mPageTable)->~PMRVector<u8*>();
        new (&mPageTable)PMRVector<u8*>();
    }
    void ObjectArrayPoolBase::Free(IndexHandle handle)
    {
        //TODO: continue workong on this, all these code needs to be reviewed
        u32 pageIdx = handle.Index >> mPageObjCountPow;
        u32 inPageIdx = handle.Index & ((1u << mPageObjCountPow) - 1u);
        if (pageIdx >= mPageTable.size())
            return false;
        u8* page = mPageTable[pageIdx];
        u8* genPtr = page + mObjectSize * inPageIdx + mGenOffset;
        THandleGen objGen = *(THandleGen*)genPtr;
        return objGen == handle.Gen;
    }
	void ObjectArrayPoolBase::_Initialize(PMRAllocator allocator, u32 pageObjCountPow, u32 pageAlign, u32 pageSize, u32 objSize, u32 genOffset)
	{
        (&mPageTable)->~PMRVector<u8*>();
        new (&mPageTable)PMRVector<u8*>(allocator);
        mPageAlign = pageAlign;
        mPageSize = pageSize;
        mObjectSize = objSize;
        mPageObjCountPow = pageObjCountPow;
        mGenOffset = genOffset;
        mFreeIndex = THandleIndex(-1);
	}
    std::pair<IndexHandle, u8*> ObjectArrayPoolBase::_Alloc()
    {
        if (mFreeIndex == THandleIndex(-1))
            Grow();
        u32 pageIdx = mFreeIndex >> mPageObjCountPow;
        u32 inPageIdx = mFreeIndex & ((1u << mPageObjCountPow) - 1u);
        u8* page = mPageTable[pageIdx];
        u8* objPtr = page + mObjectSize * inPageIdx;
        u8* genPtr = objPtr + mGenOffset;
        IndexHandle newHandle;
        newHandle.Gen = ++(*(THandleGen*)genPtr);
        newHandle.Index = mFreeIndex;
        mFreeIndex = *(THandleIndex*)objPtr;

        return std::make_pair(newHandle, objPtr);
    }
    u8* ObjectArrayPoolBase::_ToPtr(IndexHandle handle)
    {
        u32 pageIdx = handle.Index >> mPageObjCountPow;
        u32 inPageIdx = handle.Index & ((1u << mPageObjCountPow) - 1u);
        CheckDebug(pageIdx < mPageTable.size(), "invalid handle");
        u8* page = mPageTable[pageIdx];
        u8* objPtr = page + mObjectSize * inPageIdx;
        u8* genPtr = objPtr + mGenOffset;
        THandleGen objGen = *(THandleGen*)genPtr;
        CheckDebug(objGen == handle.Gen, "invalid handle");
        return objPtr;
    }
    void ObjectArrayPoolBaseGrow()
    {
    }
}
