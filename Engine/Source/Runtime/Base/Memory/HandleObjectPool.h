#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Base/Memory/MemoryDefs.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Base/Memory/ObjectHandle.h"
#include <tuple>

namespace Omni
{
    struct IndexHandlePoolBase
    {
    public:
        BASE_API inline bool IsValid(IndexHandle handle);
        BASE_API inline void Finalize();
        BASE_API inline void Free(IndexHandle handle);
        
    protected:
        BASE_API inline void _Initialize(PMRAllocator allocator, u32 pageObjCountPow, u32 pageAlign, u32 pageSize, u32 objSize, u32 genOffset);
        BASE_API inline std::tuple<IndexHandle, u8*> _Alloc();
        BASE_API inline u8* _ToPtr(IndexHandle handle);
        
    private:
        FORCEINLINE void AddPage();
    protected:
        PMRVector<u8*>      mPageTable;
        u32                 mPageAlign;
        u32                 mPageSize;
        u32                 mObjectSize;
        u32                 mPageObjCountPow;
        u32                 mGenOffset;
        u32                 mUsedCount;
        THandleIndex        mFreeIndex;
    };

    struct RawPtrHandlePoolBase
    {
    public:
        BASE_API inline bool IsValid(RawPtrHandle handle);
        BASE_API inline void Finalize();
        BASE_API inline void Free(RawPtrHandle handle);
    protected:
        BASE_API inline void _Initialize(PMRAllocator allocator, u32 pageObjCountPow, u32 pageAlign, u32 pageSize, u32 objSize, u32 genOffset);
        BASE_API inline RawPtrHandle _Alloc();
        BASE_API inline u8* _ToPtr(RawPtrHandle handle);
    private:
        FORCEINLINE void AddPage();
    protected:
        PMRVector<u8*>      mPageTable;
        u32                 mPageAlign;
        u32                 mPageSize;
        u32                 mObjectSize;
        u32                 mPageObjCountPow;
        u32                 mGenOffset;
        u32                 mUsedCount;
        u8*                 mFreePtr;
    };

    template<typename TObject>
    struct IndexObjectPool : public IndexHandlePoolBase
    {
    public:
        void Initialize(PMRAllocator allocator, u32 pageObjCount);
        std::tuple<IndexHandle, TObject*> Alloc();
        TObject* ToPtr(IndexHandle handle);
    };

    template<typename TObject>
    struct RawPtrObjectPool : public RawPtrHandlePoolBase
    {
    public:
        void Initialize(PMRAllocator allocator, u32 pageObjCount);
        std::tuple<RawPtrHandle, TObject*> Alloc();
        TObject* ToPtr(RawPtrHandle handle);
    };
}
