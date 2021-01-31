#include "Runtime/Memory/WrapperAllocator.h"
#include "Runtime/Memory/IAllocator.h"
#include "Runtime/Misc/PrivateData.h"
#include <atomic>

namespace Omni
{
	struct WrapperAllocatorImpl final : public std::pmr::memory_resource
	{
	public:
		WrapperAllocatorImpl(std::pmr::memory_resource& fallback, const char* name)
			: mFallback(fallback)
			, mName(name)
		{}
		void* do_allocate(std::size_t bytes, std::size_t alignment) override
		{
			mTotal.fetch_add(bytes, std::memory_order_relaxed);
			mUsed.fetch_add(bytes, std::memory_order_relaxed);
			return mFallback.allocate(bytes, alignment);
		}
		void do_deallocate(void* p, std::size_t bytes, std::size_t alignment) override
		{
			mUsed.fetch_sub(bytes, std::memory_order_relaxed);
			mFallback.deallocate(p, bytes, alignment);
		}
		bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override
		{
			return this == &other;
		}
	public:
		std::atomic<size_t>				mUsed;
		std::atomic<size_t>				mTotal;
		std::pmr::memory_resource&		mFallback;
		const char*						mName;
	};


	WrapperAllocator::WrapperAllocator(std::pmr::memory_resource& memResource, const char* name)
		: mData(PrivateDataType<WrapperAllocatorImpl>{}, memResource, name)
	{
	}
	PMRAllocator WrapperAllocator::GetPMRAllocator()
	{
		return PMRAllocator(mData.Ptr<WrapperAllocatorImpl>());
	}
	MemoryStats WrapperAllocator::GetStats()
	{
		WrapperAllocatorImpl& self = mData.Ref<WrapperAllocatorImpl>();
		return MemoryStats
		{
			.Used = self.mUsed,
			.Reserved = self.mTotal,
			.Name = self.mName,
		};
	}
	const char* WrapperAllocator::GetName()
	{
		return mData.Ref<WrapperAllocatorImpl>().mName;
	}
}