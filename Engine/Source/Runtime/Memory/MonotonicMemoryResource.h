#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Memory/StdMemoryResource.h"

namespace Omni
{
    class MonotonicMemoryResource final :public STD_PMR_NS::memory_resource
    {
    public:
        MonotonicMemoryResource(size_t capacity);
        ~MonotonicMemoryResource();
        void Reset();
        void* do_allocate(std::size_t bytes, std::size_t alignment) override;
        void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override;
        bool do_is_equal(const STD_PMR_NS::memory_resource& other) const noexcept override;
    private:
        size_t      mUsed;
        size_t      mCapacity;
        char*       mBuffer;
        size_t      mUseCount;
    };
}