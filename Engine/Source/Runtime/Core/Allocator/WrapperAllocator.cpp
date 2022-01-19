#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/Allocator/WrapperAllocator.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Base/Memory/MemoryWatch.h"
#include "Runtime/Base/Misc/ArrayUtils.h"
#include "Runtime/Base/Misc/PrivateData.h"
#include "Runtime/Core/System/ModuleImplHelpers.h"

#include <atomic>

namespace Omni
{
struct WrapperAllocatorPrivateData : public StdPmr::memory_resource
{
public:
    WrapperAllocatorPrivateData(StdPmr::memory_resource& fallback, const char* name) : mFallback(fallback), mName(name)
    {
    }
    void* do_allocate(std::size_t bytes, std::size_t alignment) override
    {
        mWatch.Add(AlignUpSize(bytes, alignment));
        return mFallback.allocate(bytes, alignment);
    }
    void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override
    {
        mWatch.Sub(AlignUpSize(bytes, alignment));
        mFallback.deallocate(p, bytes, alignment);
    }
    bool do_is_equal(const StdPmr::memory_resource& other) const noexcept override
    {
        return this == &other;
    }

public:
    MemoryWatch              mWatch;
    StdPmr::memory_resource& mFallback;
    const char*              mName;
};

using WrapperAllocatorImpl = PImplCombine<WrapperAllocator, WrapperAllocatorPrivateData>;

WrapperAllocator* WrapperAllocator::Create(StdPmr::memory_resource& memResource, const char* name)
{
    return InitMemFactory<WrapperAllocatorImpl>::New(memResource, name);
}

void WrapperAllocator::Destroy()
{
    WrapperAllocatorImpl* self = WrapperAllocatorImpl::GetCombinePtr(this);
    InitMemFactory<WrapperAllocator>::Delete(self);
}

PMRResource* WrapperAllocator::GetResource()
{
    WrapperAllocatorImpl* self = WrapperAllocatorImpl::GetCombinePtr(this);
    return self;
}

MemoryStats WrapperAllocator::GetStats()
{
    WrapperAllocatorImpl* self = WrapperAllocatorImpl::GetCombinePtr(this);
    MemoryStats           ret;
    ret.Name = self->mName;
    self->mWatch.Dump(ret);
    return ret;
}

const char* WrapperAllocator::GetName()
{
    WrapperAllocatorImpl* self = WrapperAllocatorImpl::GetCombinePtr(this);
    return self->mName;
}

void WrapperAllocator::Shrink()
{
}
} // namespace Omni
