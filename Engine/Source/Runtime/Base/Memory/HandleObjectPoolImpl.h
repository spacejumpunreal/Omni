#pragma once
#include "Runtime/Base/Memory/HandleObjectPool.h"
#include "Runtime/Base/Misc/AssertUtils.h"

namespace Omni
{
    template<typename TObject>
    void ObjectArrayPool<TObject>::Initialize(PMRAllocator allocator, u32 pageObjCountPow)
    {
        u32 pageObjCount = 1 << pageObjCountPow;
        u32 align = alignof(TObject);
        CheckAlways(align >= sizeof(THandleGen));
        u32 size = AlignUpSize(sizeof(TObject) + sizeof(THandleGen), align);
        u32 pageSize = size * pageObjCount;
        ObjectArrayPoolBase::Initialize(PMRAllocator allocator, pageObjCountPow, align, pageSize, size, sizeof(TObject));
    }

    template<typename TObject>
    std::pair<IndexHandle, TObject*> ObjectArrayPool<TObject>::Alloc()
    {
        auto pair = ObjectArrayPoolBase::Alloc();
        return std::make_pair<IndexHandle, TObject*>(pair.first, (TObject*)pair.second);
    }

    template<typename TObject>
    TObject* ObjectArrayPool<TObject>::ToPtr(IndexHandle handle)
    {
        return (TObject*)ObjectArrayPoolBase::ToPtr(handle);
    }
}
