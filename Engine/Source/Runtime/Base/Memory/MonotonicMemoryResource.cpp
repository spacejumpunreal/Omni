#include "Runtime/Base/BasePCH.h"
#include "Runtime/Base/Memory/MonotonicMemoryResource.h"
#include "Runtime/Base/Misc/ArrayUtils.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include <cstdlib>
#include <cstring>

namespace Omni
{
    MonotonicMemoryResource::MonotonicMemoryResource(size_t capacity)
        : mUsed(0)
        , mCapacity(capacity)
        , mBuffer((char*)malloc(capacity))
        , mUseCount(0)
    {
        Reset();
    }
    MonotonicMemoryResource::~MonotonicMemoryResource()
    {
        //CheckAlways(mUseCount == 0);
        free(mBuffer);
    }
    void MonotonicMemoryResource::Reset()
    {
        CheckAlways(mUseCount == 0);
        mUsed = 0;
        memset(mBuffer, 0xcd, mCapacity);
    }
    void* MonotonicMemoryResource::do_allocate(std::size_t bytes, std::size_t alignment)
    {
        ++mUseCount;
        size_t start = AlignUpSize(mUsed, alignment);
        size_t end = start + bytes;
        CheckAlways(end <= mCapacity);
        mUsed = end;
        return start + mBuffer;
    }
    void MonotonicMemoryResource::do_deallocate(void*, std::size_t, std::size_t)
    {
        --mUseCount;
    }
    bool MonotonicMemoryResource::do_is_equal(const StdPmr::memory_resource& other) const noexcept
    {
        return this == &other;
    }
    
}
