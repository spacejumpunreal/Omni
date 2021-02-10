#pragma once
#include "Runtime/Omni.h"
#include <memory_resource>

namespace Omni
{
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