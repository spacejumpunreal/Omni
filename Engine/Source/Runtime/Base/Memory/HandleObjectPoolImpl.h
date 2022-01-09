#pragma once
#include "Runtime/Base/Memory/HandleObjectPool.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base//Misc/ArrayUtils.h"

namespace Omni
{
    /**
    * IndexObjectPool
    */
    template<typename TObject>
    void IndexObjectPool<TObject>::Initialize(PMRAllocator allocator, u32 pageObjCountPow)
    {
        u32 pageObjCount = 1 << pageObjCountPow;
        u32 align = alignof(TObject);
        CheckAlways(align >= sizeof(THandleGen));
        u32 size = AlignUpSize((u32)(sizeof(TObject) + sizeof(THandleGen)), align);
        u32 pageSize = size * pageObjCount;
        IndexHandlePoolBase::_Initialize(allocator, pageObjCountPow, align, pageSize, size, sizeof(TObject));
    }

    template<typename TObject>
    std::tuple<IndexHandle, TObject*> IndexObjectPool<TObject>::Alloc()
    {
        IndexHandle handle;
        u8* ptr;
        std::tie(handle, ptr) = IndexHandlePoolBase::_Alloc();
        TObject* optr = (TObject*)ptr;
        return std::tie(handle, optr);
    }

    template<typename TObject>
    TObject* IndexObjectPool<TObject>::ToPtr(IndexHandle handle)
    {
        return (TObject*)IndexHandlePoolBase::_ToPtr(handle);
    }

    /**
    * RawPtrHandle
    */
    template<typename TObject>
    void RawPtrObjectPool<TObject>::Initialize(PMRAllocator allocator, u32 pageObjCountPow)
    {
        u32 pageObjCount = 1 << pageObjCountPow;
        u32 align = alignof(TObject);
        CheckAlways(align >= sizeof(THandleGen));
        u32 size = AlignUpSize((u32)(sizeof(TObject) + sizeof(THandleGen)), align);
        u32 pageSize = size * pageObjCount;
        RawPtrHandlePoolBase::_Initialize(allocator, pageObjCountPow, align, pageSize, size, sizeof(TObject));
    }

    template<typename TObject>
    std::tuple<RawPtrHandle, TObject*> RawPtrObjectPool<TObject>::Alloc()
    {
        RawPtrHandle handle;
        handle = RawPtrHandlePoolBase::_Alloc();
        TObject* optr = (TObject*)handle.Addr;
        return std::tie(handle, optr);
    }

    template<typename TObject>
    TObject* RawPtrObjectPool<TObject>::ToPtr(RawPtrHandle handle)
    {
        return (TObject*)RawPtrHandlePoolBase::_ToPtr(handle);
    }
}
