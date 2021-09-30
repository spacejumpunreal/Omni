#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Base/Memory/StdMemoryResource.h"

namespace Omni
{
	template<typename T>
	using PMRAllocatorT = StdPmr::polymorphic_allocator<T>;
	using PMRAllocator = StdPmr::polymorphic_allocator<std::byte>;
	using PMRResource = StdPmr::memory_resource;

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
#include "Runtime/Base/Memory/MemoryKind.inl"
#undef MEMORY_KIND
	};
}

