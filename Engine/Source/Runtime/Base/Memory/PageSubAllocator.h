#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Base/Memory/MemoryDefs.h"

namespace Omni
{
class PageSubAllocator
{
public:
    BASE_API static PageSubAllocator* Create(u32 pageSize, PMRAllocator selfAlloc, PMRAllocator pageAlloc);
    BASE_API PageSubAllocator(u32 pageSize, PMRAllocator selfAlloc, PMRAllocator pageAlloc); // construct on the stack
    BASE_API ~PageSubAllocator();

    BASE_API u8* AllocBytes(u32 bytes, u32 align);
    template<typename T>
    T* AllocArray(u32 count);
    template<typename T, typename... Args>
    T* New(Args... args);

    BASE_API void        Cleanup();
    BASE_API static void Destroy(void* allocator);

private:
    u8*              mAvailPtr;
    u8*              mAvailEnd;
    PMRVector<void*> mPages;
    PMRAllocator     mPageAlloc;
    u32              mPageSize;
};

template<typename T>
T* PageSubAllocator::AllocArray(u32 count)
{
    return (T*)AllocBytes(sizeof(T) * count, alignof(T));
}

template<typename T, typename... Args>
T* PageSubAllocator::New(Args... args)
{
    void* ptr = AllocBytes(sizeof(T), alignof(T));
    new (ptr) T(std::forward<T>(args)...);
    return (T*)ptr;
}

} // namespace Omni
