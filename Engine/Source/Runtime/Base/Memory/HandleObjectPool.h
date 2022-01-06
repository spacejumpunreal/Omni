#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Base/Memory/MemoryDefs.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Base/Memory/ObjectHandle.h"

namespace Omni
{
    struct ObjectArrayPoolBase
    {
    public:
        inline bool IsValid(IndexHandle handle);
        inline void Finalize();
        inline void Free(IndexHandle handle);
        
    protected:
        inline void _Initialize(PMRAllocator allocator, u32 pageObjCountPow, u32 pageAlign, u32 pageSize, u32 objSize, u32 genOffset);
        inline std::pair<IndexHandle, u8*> _Alloc();
        inline u8* _ToPtr(IndexHandle handle);

        inline void Grow();

    protected:
        PMRVector<u8*>      mPageTable;
        u32                 mPageAlign;
        u32                 mPageSize;
        u32                 mObjectSize;
        u32                 mPageObjCountPow;
        u32                 mGenOffset;
        THandleIndex        mFreeIndex;
    };

    template<typename TObject>
    struct ObjectArrayPool : public ObjectArrayPoolBase
    {
    public:
        void Initialize(PMRAllocator allocator, u32 pageObjCount);
        std::pair<IndexHandle, TObject*> Alloc();
        TObject* ToPtr(IndexHandle handle);
    };
}
