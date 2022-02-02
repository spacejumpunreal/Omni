#include "Runtime/Base/BasePCH.h"
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Base/Memory/PageSubAllocator.h"
#include "Runtime/Base/Misc/ArrayUtils.h"
#include "Runtime/Base/Misc/AssertUtils.h"

namespace Omni
{
constexpr u32 kPageAlign = 4096;

PageSubAllocator* PageSubAllocator::Create(u32 pageSize, PMRAllocator selfAlloc, PMRAllocator pageAlloc)
{
    return selfAlloc.new_object<PageSubAllocator>(pageSize, selfAlloc, pageAlloc);
}

PageSubAllocator::PageSubAllocator(u32 pageSize, PMRAllocator selfAlloc, PMRAllocator pageAlloc)
    : mAvailPtr(nullptr)
    , mAvailEnd(nullptr)
    , mPages(32, selfAlloc)
    , mPageAlloc(pageAlloc)
    , mPageSize(pageSize)
{
    mPages.resize(0);
    CheckAlways(mPageSize % kPageAlign == 0, "pageSize better be page aligned");
}
PageSubAllocator::~PageSubAllocator()
{
}
u8* PageSubAllocator::AllocBytes(u32 bytes, u32 align)
{
    CheckDebug(bytes < mPageSize);
    u8* nextAvail = (u8*)AlignUpPtr(mAvailPtr, align) + bytes;
    if (nextAvail > mAvailEnd)
    {
        mAvailPtr = (u8*)mPageAlloc.allocate_bytes(mPageSize, kPageAlign);
        mPages.push_back(mAvailPtr);
        mAvailEnd = mAvailPtr + mPageSize;
        nextAvail = (u8*)AlignUpPtr(mAvailPtr, align) + bytes;
    }
    u8* ret = mAvailPtr;
    mAvailPtr = nextAvail;
    return ret;
}
void PageSubAllocator::Cleanup()
{
    for (void* page : mPages)
    {
        mPageAlloc.deallocate_bytes(page, mPageSize, kPageAlign);
    }
}
void PageSubAllocator::Destroy(void* alloc)
{
    auto self = (PageSubAllocator*)alloc;
    PMRAllocator selfAlloc = self->mPages.get_allocator();
    self->Cleanup();
    selfAlloc.delete_object(self);
}
} // namespace Omni
