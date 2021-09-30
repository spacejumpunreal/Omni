#pragma once
#include "Runtime/Prelude/Omni.h"
#define ARRAY_LENGTH(a) (sizeof(a) / sizeof(a[0]))

namespace Omni
{
	template<typename T>
	static constexpr FORCEINLINE T AlignUpSize(T size, T align)
	{
		return (size + align - 1) & (~(align - 1));
	}
	template<typename T>
	static FORCEINLINE void* AlignUpPtr(void* ptr, T align)
	{
		return (void*)AlignUpSize((u64)ptr, (u64)align);
	}
	template<typename T>
	bool IsAligned(void* ptr, T align)
	{
		unsigned long long p = reinterpret_cast<unsigned long long>(ptr);
		return (p & (unsigned long long)(align - 1)) == 0;
	}
	template<typename T>
	constexpr bool IsPow2(T x)
	{
		return (x & (x - 1)) == 0;
	}
	inline void FillWithPattern(void* ptr, size_t size, u32 pattern)
	{
		if (size == 0)
			return;
		u8* bp = (u8*)ptr;
		u64 u64Ptr = (u64)ptr;
		u32* alignedPtr = (u32*)AlignUpSize(u64Ptr, 4ull);
		
		u32 offset = ((u64)ptr) & (sizeof(u32) - 1);
		u32 unAlignedBytes = sizeof(u32) - offset;
		for (u32 i = 0; i < unAlignedBytes && i < size; ++i)
		{
			bp[i] = (u8)(pattern >> (offset + i * 8));
		}
		size -= unAlignedBytes;
		while (size > sizeof(u32))
		{
			*alignedPtr++ = pattern;
			size -= sizeof(u32);
		}
		u8* unalignedPtr = (u8*)alignedPtr;
		for (u32 i = 0; i < size; ++i)
		{
			unalignedPtr[i] = (u8)(pattern >> (i * 8));
		}
	}
}
