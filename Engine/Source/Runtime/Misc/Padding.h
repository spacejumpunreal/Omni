#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Platform/PlatformDefs.h"

namespace Omni
{
	template<typename T, size_t PadToSize, size_t AlignAs>
	struct alignas(AlignAs) Padded
	{
	public:
		static constexpr size_t PaddingBytes = PadToSize - sizeof(T);
		static_assert(PaddingBytes >= 0);
	public:
		T		Data;
		char	Padding[PaddingBytes];
	};

	template<typename T>
	using CacheAlign = Padded<T, CPU_CACHE_LINE_SIZE, CPU_CACHE_LINE_SIZE>;
}

