#pragma once
#include "Runtime/Omni.h"
#if EXPERIMENTAL_MEMORY_RESOURCE
#include <experimental/memory_resource>
#else
#include <memory_resource>
#endif

namespace Omni
{
	template<typename T>
	using PMRAllocatorT = std::pmr::polymorphic_allocator<T>;
	using PMRAllocator = std::pmr::polymorphic_allocator<std::byte>;
	using PMRResource = std::pmr::memory_resource;

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