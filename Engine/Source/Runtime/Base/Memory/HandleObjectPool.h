#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Base/Memory/MemoryDefs.h"
#include "Runtime/Base/Container/PMRContainers.h"

namespace Omni
{
    struct ObjectArrayPool
    {
    public:
        void Initialize(PMRAllocator allocator, size_t objSize, size_t objAlign, size_t pageCount);
        void Finalize();
        u32 Alloc();
        void Free(u32);
        template<typename TObject>
        TObject* At(u32 idx);
    protected:
        PMRVector<void*> mPageTable;

    };
}
