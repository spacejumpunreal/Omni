#include "Runtime/Memory/CacheLineAllocator.h"
#include "Runtime/Concurrency/IThreadLocal.h"
#include "Runtime/Concurrency/SpinLock.h"
#include "Runtime/Math/CompileTime.h"
#include "Runtime/Memory/MemoryWatch.h"
#include "Runtime/Misc/ArrayUtils.h"
#include "Runtime/Misc/Padding.h"
#include "Runtime/Platform/PlatformAPIs.h"
#include "Runtime/Platform/PlatformDefs.h"

#include <atomic>

namespace Omni
{
	//mmap pages
	//scheme:1 core write, other cores read, always cache align to avoid false sharing
	//alloc: add ref count(almost always current thread), always page align
	//dealloc: sub ref count
	//data:
	//	per thread:
	//		- pointer, 1 page
	//	global:
	//		- free list of pages
	//		- page granulity counters

	struct CacheLinePerThreadData;
	struct CacheLinePageHeader;

	struct CacheLinePerThreadData
	{
	public:
		CacheLinePageHeader*	Page;
		u32						UsedCachelines;
	public:
		bool IsClean() { return Page == nullptr && UsedCachelines == 0; }
	};
	struct CacheLinePageHeader
	{
	public:
		static FORCEINLINE CacheLinePageHeader& GetHeader(void* p);
		CacheLinePageHeader*& GetNext() { return Top.Data.Next; }
		bool IsAvailable() { return Top.Data.AcquireCount.load(std::memory_order_relaxed) == ReleaseCount.Data.load(std::memory_order_relaxed); }
	public:
		struct TopStruct
		{
			//u64						Guard;
			std::atomic<size_t>		AcquireCount;
			CacheLinePageHeader*	Next;
			TopStruct()
				//: Guard(0xDEADBEEFDEADBEEFull)
				: AcquireCount(0)
				, Next(nullptr)
			{}
		};
		CacheAlign<TopStruct>				Top;
		CacheAlign<std::atomic<size_t>>		ReleaseCount;
	};

	OMNI_MSVC_DISABLE_WARNING(4324);

	struct CacheLineAllocatorPrivate final : public STD_PMR_NS::memory_resource
	{
	public:
		static constexpr u32 PageSize = CPU_CACHE_LINE_SIZE * 1024;
		static constexpr u32 CacheLinesPerPage = PageSize / CPU_CACHE_LINE_SIZE;
		static constexpr u32 CacheLineSizeShift = (u32)CompileTimeLog2(CPU_CACHE_LINE_SIZE);
		static constexpr u32 HeaderCacheLines = sizeof(CacheLinePageHeader) / CPU_CACHE_LINE_SIZE;
	public:
		CacheLineAllocatorPrivate()
			: mPendingListHead(nullptr)
			, mPendingListTail(nullptr)
		{}
		FORCEINLINE static u32 Size2Cachelines(size_t sz);
		void Shrink();
		void* do_allocate(std::size_t bytes, std::size_t alignment) override;
		void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override;
		bool do_is_equal(const STD_PMR_NS::memory_resource& other) const noexcept override;

	public:
		CacheLinePageHeader*				mPendingListHead;
		CacheLinePageHeader*				mPendingListTail;
		SpinLock							mLock;
		CacheAlign<MemoryWatch>				mWatch;
	};

	OMNI_RESET_WARNING();

	//global
	OMNI_DECLARE_THREAD_LOCAL(CacheLinePerThreadData, gCacheLinePerThreadData);

	CacheLinePageHeader& CacheLinePageHeader::GetHeader(void* addr)
	{
		u64 addr64 = (u64)addr;
		u64 p64 = addr64 & ~(u64)(CacheLineAllocatorPrivate::PageSize - 1);
		return *(CacheLinePageHeader*)p64;
	}

	u32 CacheLineAllocatorPrivate::Size2Cachelines(size_t sz)
	{
		return ((u32)AlignUpSize(sz, CPU_CACHE_LINE_SIZE)) >> CacheLineSizeShift;
	}
	void CacheLineAllocatorPrivate::Shrink()
	{
		mLock.Lock();
		mPendingListTail = nullptr;
		size_t pages = 0;
		for (CacheLinePageHeader** p = &mPendingListHead; *p != nullptr;)
		{
			if ((*p)->IsAvailable())
			{
				CacheLinePageHeader* op = *p;
				*p = (*p)->GetNext();
				FreePages(op, PageSize);
				++pages;
			}
			else
			{
				mPendingListTail = *p;
				p = &(*p)->GetNext();
			}
		}
		mWatch.Data.Sub(PageSize * pages);
		mLock.Unlock();
	}

	void* CacheLineAllocatorPrivate::do_allocate(std::size_t bytes, std::size_t)
	{
		u32 lines = Size2Cachelines(bytes);
		CheckDebug(lines + HeaderCacheLines <= CacheLinesPerPage);
		CacheLinePerThreadData& gcptd = gCacheLinePerThreadData.GetRaw();
		if (gcptd.Page != nullptr && gcptd.UsedCachelines + lines <= CacheLinesPerPage)
		{
			u8* ret = ((u8*)gcptd.Page) + (((u64)gcptd.UsedCachelines) << CacheLineSizeShift);
			gcptd.UsedCachelines += lines;
			gcptd.Page->Top.Data.AcquireCount.fetch_add(1, std::memory_order_relaxed); //not shared with others(not on the pending list)
			return ret;
		}
		CacheLinePageHeader* avail = nullptr;
		mLock.Lock();
		{
			CacheLinePageHeader* p;
			CacheLinePageHeader* pp;
			for (p = mPendingListHead, pp = nullptr; p != nullptr;)
			{
				if (p->IsAvailable())
				{
					avail = p;
					if (pp)
						pp->GetNext() = p->GetNext();
					else
						mPendingListHead = p->GetNext();
					if (p->GetNext() == nullptr)//only touch this if we had touched tail node
						mPendingListTail = pp;
					p->GetNext() = nullptr;
					break;
				}
				pp = p;
				p = p->GetNext();
			}
			if (gcptd.Page != nullptr)
			{
				CheckDebug(gcptd.Page->GetNext() == nullptr);
				if (mPendingListTail)
				{
					mPendingListTail->GetNext() = gcptd.Page;
					mPendingListTail = gcptd.Page;
				}
				else
				{
					CheckDebug(mPendingListHead == nullptr);
					mPendingListHead = mPendingListTail = gcptd.Page;
				}
			}
		}
		mLock.Unlock();
		if (avail == nullptr)
		{
			//allocate page
			avail = (CacheLinePageHeader*)AllocPages(PageSize);
			new (avail)CacheLinePageHeader();
			mWatch.Data.Add(PageSize);
		}
		avail->Top.Data.AcquireCount.store(1, std::memory_order_relaxed);
		avail->ReleaseCount.Data.store(0, std::memory_order_relaxed); //these will complete before put on pending list(after enter lock)
		gcptd.Page = avail;
		gcptd.UsedCachelines = HeaderCacheLines + lines;
		return (u8*)(&gcptd.Page[1]);
	}
	void CacheLineAllocatorPrivate::do_deallocate(void* p, std::size_t, std::size_t)
	{
		//finish operation on mem before release(already on the list)
		CacheLinePageHeader& hdr = CacheLinePageHeader::GetHeader(p);
		hdr.ReleaseCount.Data.fetch_add(1, std::memory_order_acq_rel);
	}
	bool CacheLineAllocatorPrivate::do_is_equal(const STD_PMR_NS::memory_resource& other) const noexcept
	{
		return this == &other;
	}

	CacheLineAllocator::CacheLineAllocator()
		: mData(PrivateDataType<CacheLineAllocatorPrivate>{})
	{
	}

	CacheLineAllocator::~CacheLineAllocator()
	{
		mData.DestroyAs<CacheLineAllocatorPrivate>();
	}

	PMRResource* CacheLineAllocator::GetResource()
	{
		return mData.Ptr<CacheLineAllocatorPrivate>();
	}

	MemoryStats CacheLineAllocator::GetStats()
	{
		MemoryStats ret;
		CacheLineAllocatorPrivate& self = mData.Ref<CacheLineAllocatorPrivate>();
		ret.Name = GetName();
		self.mWatch.Data.Dump(ret);
		return ret;
	}

	const char* CacheLineAllocator::GetName()
	{
		return "CacheLineAllocator";
	}

	void CacheLineAllocator::Shrink()
	{
		CacheLineAllocatorPrivate& self = mData.Ref<CacheLineAllocatorPrivate>();
		self.Shrink();
	}

	void CacheLineAllocator::ThreadInitialize()
	{
		CheckAlways(
			gCacheLinePerThreadData->Page == nullptr &&
			gCacheLinePerThreadData->UsedCachelines == 0
		);
	}

	void CacheLineAllocator::ThreadFinalize()
	{
		CacheLinePerThreadData& gcptd = gCacheLinePerThreadData.GetRaw();
		if (gcptd.Page)
		{
			CacheLineAllocatorPrivate& self = mData.Ref<CacheLineAllocatorPrivate>();
			self.mLock.Lock();
			if (self.mPendingListTail)
			{
				self.mPendingListTail->GetNext() = gcptd.Page;
				self.mPendingListTail = gcptd.Page;
			}
			else
			{
				self.mPendingListHead = self.mPendingListTail = gcptd.Page;
			}
			self.mLock.Unlock();
			gcptd.Page = nullptr;
			gcptd.UsedCachelines = 0;
		}
	}
}
