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
        u32 pageIdx = handle.Index >> mPageObjCountPow;
        u32 inPageIdx = handle.Index & ((1u << mPageObjCountPow) - 1u);
        if (pageIdx >= mPageTable.size())
            return false;
        u8* page = mPageTable[pageIdx];
        u8* genPtr = page + mObjectSize * inPageIdx + mGenOffset;
        THandleGen objGen = *(THandleGen*)genPtr;
        return objGen == handle.Gen;
    }
    void IndexHandlePoolBase::Finalize()
    {
        for (u32 iPage = 0; iPage < mPageTable.size(); ++iPage)
        {
            mPageTable.get_allocator().deallocate_bytes(mPageTable[iPage], mPageSize, mPageAlign);
        }
        (&mPageTable)->~PMRVector<u8*>();
        new (&mPageTable)PMRVector<u8*>(GetDummyMemoryResource());
    }
    void IndexHandlePoolBase::Free(IndexHandle handle)
    {
        u32 pageIdx = handle.Index >> mPageObjCountPow;
        u32 inPageIdx = handle.Index & ((1u << mPageObjCountPow) - 1u);
        CheckDebug(pageIdx < mPageTable.size());
        u8* page = mPageTable[pageIdx];
        u8* genPtr = page + mObjectSize * inPageIdx + mGenOffset;
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
        newHandle.Index = mFreeIndex;
        mFreeIndex = *(THandleIndex*)objPtr;
        return std::tie(newHandle, objPtr);
    }
    u8* IndexHandlePoolBase::_ToPtr(IndexHandle handle)
    {
        u32 pageIdx = handle.Index >> mPageObjCountPow;
        u32 inPageIdx = handle.Index & ((1u << mPageObjCountPow) - 1u);
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
    FORCEINLINE std::tuple<u32, u32> IndexHandlePoolBase::DecodeIndex(THandleIndex idx)
    {
        u32 pageIdx = idx >> mPageObjCountPow;
        u32 inPageIdx = idx & ((1u << mPageObjCountPow) - 1u);
        return std::tie(pageIdx, inPageIdx);
    }


    /**
    * RawPtrHandle
    */
    bool RawPtrHandlePoolBase::IsValid(RawPtrHandle handle)
    {
        return handle.AddrGen.Gen == *(u16*)(handle.AddrGen.Addr + mGenOffset);
    }
    void RawPtrHandlePoolBase::Finalize()
    {
        for (u32 iPage = 0; iPage < mPageTable.size(); ++iPage)
        {
            mPageTable.get_allocator().deallocate_bytes(mPageTable[iPage], mPageSize, mPageAlign);
        }
        (&mPageTable)->~PMRVector<u8*>();
        new (&mPageTable)PMRVector<u8*>(GetDummyMemoryResource());
    }
    void RawPtrHandlePoolBase::Free(RawPtrHandle handle)
    {
        *(u8**)(handle.Ptr) = mFreePtr;
        CheckDebug(*(u16*)(handle.AddrGen.Addr + mGenOffset) == handle.AddrGen.Gen, "invalid handle");
        ++*(u16*)(handle.AddrGen.Addr + mGenOffset);
        mFreePtr = handle.Ptr;

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
        mFreePtr = NullPtrHandle.Ptr;
    }
    RawPtrHandle RawPtrHandlePoolBase::_Alloc()
    {
        if (mFreePtr == NullPtrHandle.Ptr)
            AddPage();
        RawPtrHandle ret;
        u8* newPtr = mFreePtr;
        ret.AddrGen.Addr = (u64)newPtr;
        mFreePtr = *(u8**)mFreePtr;
        ret.AddrGen.Gen = ++*(u16*)(newPtr + mGenOffset);
        return ret;
    }
    u8* RawPtrHandlePoolBase::_ToPtr(RawPtrHandle handle)
    {
        CheckDebug(handle.AddrGen.Gen == *(u16*)(handle.AddrGen.Addr + mGenOffset), "invalid handle");
        return (u8*)handle.AddrGen.Addr;
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
