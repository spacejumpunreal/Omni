#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/Allocator/SNAllocator.h"
#include "Runtime/Base/Memory/MemoryWatch.h"
#include "Runtime/Base/Misc/ArrayUtils.h"
#include "Runtime/Core/Allocator/SNMallocWrapper.h"

#define OMNI_SNALLOCATOR_TRACK_EVERY_ALLOC 1

namespace Omni
{
	static constexpr bool TrashAllocatedMemory = true;
	static constexpr bool TrashFreedMemory = true;
	static constexpr bool CheckSizeOnDealloc = true;
	static constexpr u32 FillPatternAllocated = 0x12345678;
	static constexpr u32 FillPatternFreed = 0xDEADBEEF;

	struct SNAllocatorPrivate final : public StdPmr::memory_resource
	{
	public:
		void* do_allocate(std::size_t bytes, std::size_t alignment) override
		{
			size_t alignedSize = AlignUpSize(bytes, alignment);
			snmalloc::Alloc* alloc = snmalloc::ThreadAlloc::get_reference();
			void* p = alloc->alloc(alignedSize);
#if OMNI_SNALLOCATOR_TRACK_EVERY_ALLOC
			{
				mTraceLock.lock();
				++mTraceIndex;
		#if 0
				if (mTraceIndex == 50)
				{
					Omni::OmniDebugBreak();
				}
		#endif
				mTraceLivingAllocations.insert(std::make_pair(p, AllocInfo{ .Size = (u32)alignedSize, .Index = mTraceIndex }));
				mTraceLock.unlock();
			}
#endif
			if constexpr (TrashAllocatedMemory)
			{
				FillWithPattern(p, alignedSize, FillPatternAllocated);
			}
			mWatch.Add(alignedSize);
			return p;
		}
		void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override
		{
			size_t alignedSize = AlignUpSize(bytes, alignment);
			mWatch.Sub(alignedSize);
			snmalloc::Alloc* alloc = snmalloc::ThreadAlloc::get_reference();
			if constexpr (CheckSizeOnDealloc)
			{
				alloc->check_size(p, alignedSize);
			}
			if constexpr (TrashFreedMemory)
			{
				FillWithPattern(p, alignedSize, FillPatternFreed);
			}
			alloc->dealloc(p);
#if OMNI_SNALLOCATOR_TRACK_EVERY_ALLOC
			{
				mTraceLock.lock();
				++mTraceIndex;
				CheckAlways(mTraceLivingAllocations.erase(p) == 1);
				mTraceLock.unlock();
			}
#endif
		}
		bool do_is_equal(const StdPmr::memory_resource& other) const noexcept override
		{
			return this == &other;
		}
	public:
		MemoryWatch						mWatch;

#if OMNI_SNALLOCATOR_TRACK_EVERY_ALLOC
	public:
		struct AllocInfo
		{
			u32 Size;
			u32 Index;
		};
		std::mutex mTraceLock;
		u32 mTraceIndex;
		std::unordered_map<void*, AllocInfo> mTraceLivingAllocations;
#endif
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
	{
		//SNAllocatorPrivate& self = mData.Ref<SNAllocatorPrivate>();
		//(void)self;
		//Omni::OmniDebugBreak();
	}
}

