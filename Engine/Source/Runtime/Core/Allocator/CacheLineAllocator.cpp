#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/Allocator/CacheLineAllocator.h"
#include "Runtime/Prelude/PlatformDefs.h"
#include "Runtime/Base/Math/CompileTime.h"
#include "Runtime/Base/Memory/MemoryWatch.h"
#include "Runtime/Base/Misc/ArrayUtils.h"
#include "Runtime/Base/Misc/Padding.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Base/Misc/PlatformAPIs.h"
#include "Runtime/Base/MultiThread/IThreadLocal.h"
#include "Runtime/Base/MultiThread/LockfreeContainer.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Concurrency/LockfreeNodeCache.h"
#include "Runtime/Core/System/ModuleImplHelpers.h"



#include <atomic>

namespace Omni
{
// forward decl
struct CacheLinePageHeader;

// declarations
struct CacheLinePerThreadData
{
public:
    static constexpr u32 LocalPageCount = 4;

public:
    // too verbose to declare/implement methods in this class, just treat this as POD
    bool IsClean()
    {
        return Index == 0 && UsedCachelines == 0;
    }

public:
    u32                  UsedCachelines; // for current active page
    u32                  Index;
    CacheLinePageHeader* Pages[LocalPageCount];
};

struct CacheLinePageHeader
{
public:
    static FORCEINLINE CacheLinePageHeader& GetHeader(void* addr);
    bool                                    IsAvailable()
    {
        return AcquireCount.Data.load(std::memory_order_relaxed) == ReleaseCount.Data.load(std::memory_order_relaxed);
    }

public:
    CacheAligned<std::atomic<size_t>> AcquireCount;
    CacheAligned<std::atomic<size_t>> ReleaseCount;
};

OMNI_PUSH_WARNING()
OMNI_SUPPRESS_WARNING_PADDED_DUE_TO_ALIGNMENT()
/*
    alloc algorithm:
    1. do bump allocation on local page, if exhausted, try next entry in ring buffer
    2. when using new entry(page), check if is totally free, use if free, otherwise put it on global list for inspection
    3. loop over global queue(mPendingPages) to find free page, if no avail, mmap new one
    free algorithm:
    1. decrease use count foro page
*/
struct CacheLineAllocatorPrivate : public StdPmr::memory_resource
{
public:
    static constexpr u32 PageSize = CPU_CACHE_LINE_SIZE * 1024;
    static constexpr u32 CacheLinesPerPage = PageSize / CPU_CACHE_LINE_SIZE;
    static constexpr u32 CacheLineSizeShift = (u32)CompileTimeLog2(CPU_CACHE_LINE_SIZE);
    static constexpr u32 HeaderCacheLines = sizeof(CacheLinePageHeader) / CPU_CACHE_LINE_SIZE;

public:
    CacheLineAllocatorPrivate();
    FORCEINLINE static u32 Size2Cachelines(size_t sz);
    void                   Shrink();
    void*                  do_allocate(std::size_t bytes, std::size_t alignment) override;
    void                   do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override;
    bool                   do_is_equal(const StdPmr::memory_resource& other) const noexcept override;

public:
    LockfreeQueue<LockfreeNodeCache> mPendingPages;
    CacheAligned<MemoryWatch>        mWatch;
};
OMNI_POP_WARNING()

using CacheLineAllocatorImpl = PImplCombine<CacheLineAllocator, CacheLineAllocatorPrivate>;


// global data
OMNI_DECLARE_THREAD_LOCAL(CacheLinePerThreadData, gCacheLinePerThreadData);

// CacheLinePageHeader impl
CacheLinePageHeader& CacheLinePageHeader::GetHeader(void* addr)
{
    u64 addr64 = (u64)addr;
    u64 p64 = addr64 & ~(u64)(CacheLineAllocatorPrivate::PageSize - 1);
    return *(CacheLinePageHeader*)p64;
}

// CacheLineAllocatorPrivate impl
CacheLineAllocatorPrivate::CacheLineAllocatorPrivate() : mPendingPages(1)
{
}

u32 CacheLineAllocatorPrivate::Size2Cachelines(size_t sz)
{
    return ((u32)AlignUpSize(sz, CPU_CACHE_LINE_SIZE)) >> CacheLineSizeShift;
}

void CacheLineAllocatorPrivate::Shrink()
{
    MemoryModule& mm = MemoryModule::Get();
    while (true)
    {
        LockfreeNode* node = mPendingPages.Dequeue();
        if (!node)
            return;
        mm.Munmap(node->Data[0], PageSize);
        LockfreeNodeCache::Free(node);
    }
}

void* CacheLineAllocatorPrivate::do_allocate(std::size_t bytes, std::size_t)
{
    u32 lines = Size2Cachelines(bytes);
    CheckDebug(lines + HeaderCacheLines <= CacheLinesPerPage);
    CacheLinePerThreadData& ld = gCacheLinePerThreadData.GetRaw();
    if (ld.UsedCachelines + lines > CacheLinesPerPage)
    {
        u32 ni = (ld.Index + 1) % CacheLinePerThreadData::LocalPageCount;
        if (!ld.Pages[ni]->IsAvailable())
        {
            LockfreeNode* tail = nullptr;
            LockfreeNode* head = nullptr;
            LockfreeNode* avail = nullptr;
            while (!avail)
            {
                LockfreeNode* node = mPendingPages.Dequeue();
                if ((!node))
                    break;
                CacheLinePageHeader* h = (CacheLinePageHeader*)node->Data[0];
                if (h->IsAvailable())
                    avail = node;
                if (head == nullptr)
                    head = tail = node;
                else
                {
                    tail->Next = node;
                    tail = node;
                }
                if (avail)
                    break;
            }
            if (!avail)
            {
                MemoryModule&        mm = MemoryModule::Get();
                CacheLinePageHeader* p = (CacheLinePageHeader*)mm.Mmap(CacheLineAllocatorPrivate::PageSize,
                                                                       CacheLineAllocatorPrivate::PageSize);
                new (p) CacheLinePageHeader();
                avail = LockfreeNodeCache::Alloc();
                avail->Data[0] = p;
                avail->Next = nullptr;
                if (head == nullptr)
                    head = tail = avail;
                else
                {
                    tail->Next = avail;
                    tail = avail;
                }
            }
            CacheLinePageHeader* t = ld.Pages[ld.Index];
            ld.Pages[ld.Index] = (CacheLinePageHeader*)avail->Data[0];
            avail->Data[0] = t;
            CheckDebug(avail->Next == nullptr);
            CheckDebug((((u64)head) & 0xf) == 0);
            mPendingPages.Enqueue(head, tail);
        }
        else
        {
            ld.Index = ni;
        }
        ld.UsedCachelines = HeaderCacheLines;
    }
    ld.Pages[ld.Index]->AcquireCount.Data.fetch_add(1, std::memory_order_relaxed);
    char* p = ((char*)ld.Pages[ld.Index]) + ((u64)ld.UsedCachelines << CacheLineAllocatorPrivate::CacheLineSizeShift);
    ld.UsedCachelines += lines;
    return p;
}

void CacheLineAllocatorPrivate::do_deallocate(void* p, std::size_t, std::size_t)
{
    CacheLinePageHeader& header = CacheLinePageHeader::GetHeader(p);
    header.ReleaseCount.Data.fetch_add(1, std::memory_order_release);
}

bool CacheLineAllocatorPrivate::do_is_equal(const StdPmr::memory_resource& other) const noexcept
{
    return this == &other;
}

// CacheLineAllocator impl
CacheLineAllocator* CacheLineAllocator::Create()
{
    return InitMemFactory<CacheLineAllocatorImpl>::New();
}

void CacheLineAllocator::Destroy()
{
    CacheLineAllocatorImpl* self = CacheLineAllocatorImpl::GetCombinePtr(this);
    InitMemFactory<CacheLineAllocatorImpl>::Delete(self);
}

PMRResource* CacheLineAllocator::GetResource()
{
    CacheLineAllocatorImpl* self = CacheLineAllocatorImpl::GetCombinePtr(this);
    return self;
}

MemoryStats CacheLineAllocator::GetStats()
{
    MemoryStats                ret;
    CacheLineAllocatorImpl*    self = CacheLineAllocatorImpl::GetCombinePtr(this);
    ret.Name = GetName();
    self->mWatch.Data.Dump(ret);
    return ret;
}

const char* CacheLineAllocator::GetName()
{
    return "CacheLineAllocator";
}

void CacheLineAllocator::Shrink()
{
    CacheLineAllocatorPrivate* self = CacheLineAllocatorImpl::GetCombinePtr(this);
    self->Shrink();
}

void CacheLineAllocator::ThreadInitialize()
{
    CheckAlways(gCacheLinePerThreadData->IsClean());
    MemoryModule& mm = MemoryModule::Get();
    gCacheLinePerThreadData->Index = 0;
    gCacheLinePerThreadData->UsedCachelines = CacheLineAllocatorPrivate::HeaderCacheLines;
    for (u32 i = 0; i < CacheLinePerThreadData::LocalPageCount; ++i)
    {
        auto p = gCacheLinePerThreadData->Pages[i] =
            (CacheLinePageHeader*)mm.Mmap(CacheLineAllocatorPrivate::PageSize, CacheLineAllocatorPrivate::PageSize);
        new (p) CacheLinePageHeader();
    }
}

void CacheLineAllocator::ThreadFinalize()
{
    CacheLineAllocatorImpl* self = CacheLineAllocatorImpl::GetCombinePtr(this);
    for (u32 i = 0; i < CacheLinePerThreadData::LocalPageCount; ++i)
    {
        LockfreeNode* node = LockfreeNodeCache::Alloc();
        node->Data[0] = gCacheLinePerThreadData->Pages[i];
        self->mPendingPages.Enqueue(node);
    }
    gCacheLinePerThreadData->Index = 0;
    gCacheLinePerThreadData->UsedCachelines = 0;
}
} // namespace Omni
