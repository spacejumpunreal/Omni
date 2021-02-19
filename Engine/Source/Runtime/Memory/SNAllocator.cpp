#include "Runtime/Memory/SNAllocator.h"
#include "Runtime/Memory/MemoryWatch.h"
#include "Runtime/Misc/ArrayUtils.h"

#pragma warning( push )
#pragma warning( disable : 4324 4127)
#include "External/snmalloc/src/snmalloc.h"
#pragma warning( pop )

namespace Omni
{
	static constexpr bool TrashAllocatedMemory = true;
	static constexpr u32 FillPattern = 0xDEADBEEF;
	struct SNAllocatorPrivate final : public STD_PMR_NS::memory_resource
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
		bool do_is_equal(const STD_PMR_NS::memory_resource& other) const noexcept override
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
