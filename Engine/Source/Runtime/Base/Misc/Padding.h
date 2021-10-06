#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Base/Misc/ArrayUtils.h"
#include "Runtime/Prelude/PlatformDefs.h"

namespace Omni
{
	template<typename T, size_t PadToSize, size_t AlignAs>
	struct alignas(AlignAs) Padded
	{
	public:
		static constexpr size_t PaddingBytes = PadToSize - sizeof(T);
	public:
		T		Data;
		char	Padding[PaddingBytes];
	};

	template<typename T>
	using CacheAligned = Padded <T, AlignUpSize(sizeof(T), CPU_CACHE_LINE_SIZE), CPU_CACHE_LINE_SIZE>;
}

