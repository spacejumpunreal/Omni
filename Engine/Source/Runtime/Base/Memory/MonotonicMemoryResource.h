#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Base/Memory/StdMemoryResource.h"

namespace Omni
{
    class MonotonicMemoryResource final :public StdPmr::memory_resource
    {
    public:
        BASE_API MonotonicMemoryResource(size_t capacity);
        BASE_API ~MonotonicMemoryResource();
        BASE_API void Reset();
        BASE_API void* do_allocate(std::size_t bytes, std::size_t alignment) override;
        BASE_API void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override;
        BASE_API bool do_is_equal(const StdPmr::memory_resource& other) const noexcept override;
    private:
        size_t      mUsed;
        size_t      mCapacity;
        char*       mBuffer;
        size_t      mUseCount;
    };
}
