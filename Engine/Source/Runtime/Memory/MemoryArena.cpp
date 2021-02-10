#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Memory/MemoryArena.h"
#include "Runtime/Misc/ArrayUtils.h"
#include "Runtime/Misc/AssertUtils.h"
#include <memory_resource>

namespace Omni
{
	MemoryArena::MemoryArena()
		: mPtr(nullptr)
		, mUsedBytes(0)
		, mTotalBytes(0)
		, mDepth(0)
	{
		memset(mOffsets, 0, sizeof(mOffsets));
	}
	void MemoryArena::Reset(u8* ptr, u32 bytes)
	{
		CheckAlways(IsAligned(ptr, Alignment));
		CheckAlways(mUsedBytes == 0 && mTotalBytes == 0 && mDepth == 0);
		mPtr = ptr;
		mTotalBytes = bytes;
	}
	u8* MemoryArena::Allocate(u32 size)
	{
		u8* ret = mPtr + mUsedBytes;
		mUsedBytes += AlignUpSize(size, Alignment);
		CheckDebug(mUsedBytes <= mTotalBytes);
		return ret;
	}
	void MemoryArena::Push()
	{
		CheckDebug(mDepth < MaxDepth);
		mOffsets[mDepth++] = mUsedBytes;
	}
	void MemoryArena::Pop()
	{
		CheckDebug(mDepth > 0);
		mUsedBytes = mOffsets[--mDepth];
	}
}
