#pragma once

#define ARRAY_LENGTH(a) (sizeof(a) / sizeof(a[0]))

namespace Omni
{
	template<typename T>
	static FORCEINLINE T RoundUpSize(T size, T align)
	{
		return (size + align - 1) & (~(align - 1));
	}
	template<typename T>
	bool IsAligned(void* ptr, T align)
	{
		unsigned long long p = reinterpret_cast<unsigned long long>(ptr);
		return (p & (unsigned long long)(align - 1)) == 0;
	}
}
