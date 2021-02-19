#pragma once
#include "Runtime/Omni.h"
#if PMR_IS_EXPERIMENTAL
#include <experimental/memory_resource>
#else
#include <memory_resource>
#endif

namespace Omni
{
	template<typename T>
	using PMRAllocatorT = STD_PMR_NS::polymorphic_allocator<T>;
	using PMRAllocator = STD_PMR_NS::polymorphic_allocator<std::byte>;
	using PMRResource = STD_PMR_NS::memory_resource;

	struct MemoryStats
	{
		u64				Used;
		u64				Peak;
		u64				Total;
		u64				Throughput;
		const char*		Name;
	};

	enum class MemoryKind : u32
	{
#define MEMORY_KIND(X) X,
#include "Runtime/Memory/MemoryKind.inl"
#undef MEMORY_KIND
	};
}