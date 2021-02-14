#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Misc/AssertUtils.h"

namespace Omni
{
	void* AllocPages(size_t size);
	void FreePages(void* mem, size_t size);

	void PauseThread();

	static constexpr u64 TaggedPointerTagBegin = 48;
	static constexpr u64 TaggedPointerTagRange = 16;
	static constexpr u64 TaggedPointerTagMask = ((1ull << TaggedPointerTagRange) - 1) << TaggedPointerTagBegin;

	template<typename T, typename U>
	T* TagPointer(T* p, U tag)
	{
		u64 pp = (u64)p;
		CheckDebug((p & TaggedPointerTagMask) == 0);
		u64 t = (u64)tag;
		pp |= (t << TaggedPointerTagBegin);
		return (T*)pp;
	}

	template<typename T, typename U>
	T* UntagPointer(T* p, U& tag)
	{
		u64 pp = (u64)p;
		tag = (pp >> TaggedPointerTagBegin) & ((1ull << TaggedPointerTagRange) - 1);
		u64 ppp = pp & ~TaggedPointerTagMask;
		return (T*)ppp;
	}
}