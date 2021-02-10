#pragma once

#define ARRAY_LENGTH(a) (sizeof(a) / sizeof(a[0]))

namespace Omni
{
	template<typename T>
	static FORCEINLINE T AlignUpSize(T size, T align)
	{
		return (size + align - 1) & (~(align - 1));
	}
	template<typename T>
	bool IsAligned(void* ptr, T align)
	{
		unsigned long long p = reinterpret_cast<unsigned long long>(ptr);
		return (p & (unsigned long long)(align - 1)) == 0;
	}
	inline void FillWithPattern(void* ptr, size_t size, u32 pattern)
	{
		u8* bp = (u8*)ptr;
		u64 u64Ptr = (u64)ptr;
		u32* alignedPtr = (u32*)AlignUpSize(u64Ptr, 4ull);
		
		u32 offset = ((u64)ptr) & 0x3;
		for (u32 i = 0; 0 < sizeof(u32) - offset && i < size; ++i)
		{
			bp[i] = (u8)(pattern >> (offset + i));
		}
		size -= (sizeof(32) - offset);
		while (size > 4)
		{
			*alignedPtr++ = pattern;
			--size;
		}
		u8* unalignedPtr = (u8*)alignedPtr;
		for (u32 i = 0; i < size; ++i)
		{
			unalignedPtr[i] = (u8)(pattern >> i);
		}
	}
}
