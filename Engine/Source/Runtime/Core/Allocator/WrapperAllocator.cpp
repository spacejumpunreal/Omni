#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/Allocator/WrapperAllocator.h"
#include "Runtime/Base/Memory/MemoryWatch.h"
#include "Runtime/Base/Misc/ArrayUtils.h"
#include "Runtime/Base/Misc/PrivateData.h"

#include <atomic>

namespace Omni
{
	struct WrapperAllocatorImpl final : public StdPmr::memory_resource
	{
	public:
		WrapperAllocatorImpl(StdPmr::memory_resource& fallback, const char* name)
			: mFallback(fallback)
			, mName(name)
		{}
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
		MemoryWatch						mWatch;
		StdPmr::memory_resource&		mFallback;
		const char*						mName;
	};
	WrapperAllocator::WrapperAllocator(StdPmr::memory_resource& memResource, const char* name)
		: mData(PrivateDataType<WrapperAllocatorImpl>{}, memResource, name)
	{
	}
	WrapperAllocator::~WrapperAllocator()
	{
		mData.DestroyAs<WrapperAllocatorImpl>();
	}
	PMRResource* WrapperAllocator::GetResource()
	{
		return mData.Ptr<WrapperAllocatorImpl>();
	}
	MemoryStats WrapperAllocator::GetStats()
	{
		WrapperAllocatorImpl& self = mData.Ref<WrapperAllocatorImpl>();
		MemoryStats ret;
		ret.Name = self.mName;
		self.mWatch.Dump(ret);
		return ret;
	}
	const char* WrapperAllocator::GetName()
	{
		return mData.Ref<WrapperAllocatorImpl>().mName;
	}
	void WrapperAllocator::Shrink()
	{}
}

