#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Base/Memory/MemoryDefs.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Base/Memory/ObjectHandle.h"
#include <tuple>

namespace Omni
{
    struct ObjectArrayPoolBase
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
        FORCEINLINE std::tuple<u32, u32> DecodeIndex(THandleIndex idx);

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
        std::tuple<IndexHandle, TObject*> Alloc();
        TObject* ToPtr(IndexHandle handle);
    };
}
