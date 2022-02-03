#include "Runtime/Base/BasePCH.h"
#include "Runtime/Base/Memory/HandleObjectPool.h"
#include "Runtime/Base/Misc/ArrayUtils.h"
#include "Runtime/Base/Misc/AssertUtils.h"

namespace Omni
{
    /**
    * IndexHandle
    */
    bool IndexHandlePoolBase::IsValid(IndexHandle handle)
    {
        u32 realIndex = handle.Index - 1;
        u32 pageIdx = realIndex >> mPageObjCountPow;
        u32 inPageIdx = realIndex & ((1u << mPageObjCountPow) - 1u);
        if (pageIdx >= mPageTable.size())
            return false;
        u8* page = mPageTable[pageIdx];
        u8* genPtr = page + mObjectSize * inPageIdx + mGenOffset;
        THandleGen objGen = *(THandleGen*)genPtr;
        return objGen == handle.Gen;
    }
    void IndexHandlePoolBase::Finalize()
    {
        CheckAlways(mUsedCount == 0);
        for (u32 iPage = 0; iPage < mPageTable.size(); ++iPage)
        {
            mPageTable.get_allocator().deallocate_bytes(mPageTable[iPage], mPageSize, mPageAlign);
        }
        (&mPageTable)->~PMRVector<u8*>();
        new (&mPageTable)PMRVector<u8*>(GetDummyMemoryResource());
    }
    void IndexHandlePoolBase::Free(IndexHandle handle)
    {
        u32 realIndex = handle.Index - 1;
        u32 pageIdx = realIndex >> mPageObjCountPow;
        u32 inPageIdx = realIndex & ((1u << mPageObjCountPow) - 1u);
        CheckDebug(pageIdx < mPageTable.size());
        u8* page = mPageTable[pageIdx];
        u8* genPtr = page + mObjectSize * inPageIdx + mGenOffset;
        --mUsedCount;
        ++*(THandleGen*)genPtr;
    }
    void IndexHandlePoolBase::_Initialize(PMRAllocator allocator, u32 pageObjCountPow, u32 pageAlign, u32 pageSize, u32 objSize, u32 genOffset)
    {
        (&mPageTable)->~PMRVector<u8*>();
        new (&mPageTable)PMRVector<u8*>(allocator);
        mPageAlign = pageAlign;
        mPageSize = pageSize;
        mObjectSize = objSize;
        mPageObjCountPow = pageObjCountPow;
        mGenOffset = genOffset;
        mUsedCount = 0;
        mFreeIndex = THandleIndex(-1);
    }
    std::tuple<IndexHandle, u8*> IndexHandlePoolBase::_Alloc()
    {
        if (mFreeIndex == THandleIndex(-1))
            AddPage();
        u32 pageIdx = mFreeIndex >> mPageObjCountPow;
        u32 inPageIdx = mFreeIndex & ((1u << mPageObjCountPow) - 1u);
        u8* page = mPageTable[pageIdx];
        u8* objPtr = page + mObjectSize * inPageIdx;
        u8* genPtr = objPtr + mGenOffset;
        IndexHandle newHandle;
        newHandle.Gen = ++(*(THandleGen*)genPtr);
        newHandle.Index = mFreeIndex + 1;
        mFreeIndex = *(THandleIndex*)objPtr;
        ++mUsedCount;
        return std::tie(newHandle, objPtr);
    }
    u8* IndexHandlePoolBase::_ToPtr(IndexHandle handle)
    {
        u32 realIndex = handle.Index - 1;
        u32 pageIdx = realIndex >> mPageObjCountPow;
        u32 inPageIdx = realIndex & ((1u << mPageObjCountPow) - 1u);
        CheckDebug(pageIdx < mPageTable.size(), "invalid handle");
        u8* page = mPageTable[pageIdx];
        u8* objPtr = page + mObjectSize * inPageIdx;
        u8* genPtr = objPtr + mGenOffset;
        THandleGen objGen = *(THandleGen*)genPtr;
        CheckDebug(objGen == handle.Gen, "invalid handle generation");
        return objPtr;
    }
    FORCEINLINE void IndexHandlePoolBase::AddPage()
    {
        u8* newPage = (u8*)mPageTable.get_allocator().allocate_bytes(mPageSize, mPageAlign);
        u32 pageObjectCount = 1u << mPageObjCountPow;
        THandleIndex last = mFreeIndex;
        THandleIndex base = u32(mPageTable.size()) << mPageObjCountPow;
        for (u32 iObject = 0; iObject < pageObjectCount; ++iObject)
        {
            *(THandleIndex*)(newPage + iObject * mObjectSize) = last;
            last = iObject + base;
        }
        mPageTable.push_back(newPage);
        mFreeIndex = last;
    }

    /**
    * RawPtrHandle
    */
    bool RawPtrHandlePoolBase::IsValid(RawPtrHandle handle)
    {
        return handle.Gen == *(u16*)(handle.Addr + mGenOffset);
    }
    void RawPtrHandlePoolBase::Finalize()
    {
        CheckAlways(mUsedCount == 0);
        for (u32 iPage = 0; iPage < mPageTable.size(); ++iPage)
        {
            mPageTable.get_allocator().deallocate_bytes(mPageTable[iPage], mPageSize, mPageAlign);
        }
        (&mPageTable)->~PMRVector<u8*>();
        new (&mPageTable)PMRVector<u8*>(GetDummyMemoryResource());
    }
    void RawPtrHandlePoolBase::Free(RawPtrHandle handle)
    {
        CheckDebug(*(u16*)(handle.Addr + mGenOffset) == handle.Gen, "invalid handle");
        *(u8**)(handle.Addr) = mFreePtr;
        ++*(u16*)(handle.Addr + mGenOffset);
        mFreePtr = (u8*)handle.Addr;
        --mUsedCount;
    }
    void RawPtrHandlePoolBase::_Initialize(PMRAllocator allocator, u32 pageObjCountPow, u32 pageAlign, u32 pageSize, u32 objSize, u32 genOffset)
    {
        (&mPageTable)->~PMRVector<u8*>();
        new (&mPageTable)PMRVector<u8*>(allocator);
        mPageAlign = pageAlign;
        mPageSize = pageSize;
        mObjectSize = objSize;
        mPageObjCountPow = pageObjCountPow;
        mGenOffset = genOffset;
        mUsedCount = 0;
        mFreePtr = (u8*)NullPtrHandle.Addr;
    }
    RawPtrHandle RawPtrHandlePoolBase::_Alloc()
    {
        if (mFreePtr == (u8*)NullPtrHandle.Addr)
            AddPage();
        RawPtrHandle ret;
        u8* newPtr = mFreePtr;
        ret.Addr = (u64)newPtr;
        mFreePtr = *(u8**)mFreePtr;
        ret.Gen = ++*(u16*)(newPtr + mGenOffset);
        ++mUsedCount;
        return ret;
    }
    u8* RawPtrHandlePoolBase::_ToPtr(RawPtrHandle handle)
    {
        CheckDebug(handle.Gen == *(u16*)(handle.Addr + mGenOffset), "invalid handle");
        return (u8*)handle.Addr;
    }
    FORCEINLINE void RawPtrHandlePoolBase::AddPage()
    {
        u8* newPage = (u8*)mPageTable.get_allocator().allocate_bytes(mPageSize, mPageAlign);
        u32 pageObjectCount = 1u << mPageObjCountPow;
        u8* last = mFreePtr;
        for (u32 iObject = 0; iObject < pageObjectCount; ++iObject)
        {
            u8* p = newPage + iObject * mObjectSize;
            *(u8**)(p) = last;
            last = p;
        }
        mPageTable.push_back(newPage);
        mFreePtr = last;
    }
}
