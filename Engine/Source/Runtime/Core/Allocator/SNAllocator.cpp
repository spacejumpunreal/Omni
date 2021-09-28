#include "CorePCH.h"
#include "Allocator/SNAllocator.h"
#include "Memory/MemoryWatch.h"
#include "Allocator/SNMallocWrapper.h"
#include "Misc/ArrayUtils.h"



namespace Omni
{
	static constexpr bool TrashAllocatedMemory = true;
	static constexpr u32 FillPattern = 0xDEADBEEF;
	struct SNAllocatorPrivate final : public StdPmr::memory_resource
	{
	public:
		void* do_allocate(std::size_t bytes, std::size_t alignment) override
		{
			size_t alignedSize = AlignUpSize(bytes, alignment);
			snmalloc::Alloc* alloc = snmalloc::ThreadAlloc::get_reference();
			void* p = alloc->alloc(alignedSize);
			if (TrashAllocatedMemory)
			{
				FillWithPattern(p, alignedSize, FillPattern);
			}
			mWatch.Add(alignedSize);
			return p;
		}
		void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override
		{
			size_t alignedSize = AlignUpSize(bytes, alignment);
			mWatch.Sub(alignedSize);
			snmalloc::ThreadAlloc::get_noncachable()->dealloc(p);
		}
		bool do_is_equal(const StdPmr::memory_resource& other) const noexcept override
		{
			return this == &other;
		}
	public:
		MemoryWatch						mWatch;
	};
	SNAllocator::SNAllocator()
		: mData(PrivateDataType<SNAllocatorPrivate>{})
	{
	}
	SNAllocator::~SNAllocator()
	{
		mData.DestroyAs<SNAllocatorPrivate>();
	}
	PMRResource* SNAllocator::GetResource()
	{
		return mData.Ptr<SNAllocatorPrivate>();
	}
	MemoryStats SNAllocator::GetStats()
	{
		MemoryStats ret;
		SNAllocatorPrivate& self = mData.Ref<SNAllocatorPrivate>();
		ret.Name = GetName();
		self.mWatch.Dump(ret);
		return ret;
	}
	const char* SNAllocator::GetName()
	{
		return "SNAllocator";
	}
	void SNAllocator::Shrink()
	{}
}

