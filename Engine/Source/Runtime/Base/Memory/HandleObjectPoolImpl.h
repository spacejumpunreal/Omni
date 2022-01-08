#pragma once
#include "Runtime/Base/Memory/HandleObjectPool.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base//Misc/ArrayUtils.h"

namespace Omni
{
    template<typename TObject>
    void ObjectArrayPool<TObject>::Initialize(PMRAllocator allocator, u32 pageObjCountPow)
    {
        u32 pageObjCount = 1 << pageObjCountPow;
        u32 align = alignof(TObject);
        CheckAlways(align >= sizeof(THandleGen));
        u32 size = AlignUpSize((u32)(sizeof(TObject) + sizeof(THandleGen)), align);
        u32 pageSize = size * pageObjCount;
        ObjectArrayPoolBase::_Initialize(allocator, pageObjCountPow, align, pageSize, size, sizeof(TObject));
    }

    template<typename TObject>
    std::tuple<IndexHandle, TObject*> ObjectArrayPool<TObject>::Alloc()
    {
        IndexHandle handle;
        u8* ptr;
        std::tie(handle, ptr) = ObjectArrayPoolBase::_Alloc();
        TObject* optr = (TObject*)ptr;
        return std::tie(handle, optr);
    }

    template<typename TObject>
    TObject* ObjectArrayPool<TObject>::ToPtr(IndexHandle handle)
    {
        return (TObject*)ObjectArrayPoolBase::_ToPtr(handle);
    }
}
