#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Misc/ArrayUtils.h"
#include "Runtime/Misc/AssertUtils.h"
#include <memory_resource>

namespace Omni
{
	class ScratchStack;

	class MemoryArenaScope
	{
	private:
		FORCEINLINE MemoryArenaScope(ScratchStack& arena, u32 depth);
	public:
		FORCEINLINE ~MemoryArenaScope();
	private:
		ScratchStack& mArena;
		u32 mDepth;
		friend class ScratchStack;
	};
	class ScratchStack
	{
	public:
		static const u32 MaxDepth = 16;
		static const u32 Alignment = OMNI_DEFAULT_ALIGNMENT;
	public:
		ScratchStack();
		void Reset(u8* ptr, u32 size);
		FORCEINLINE u8* Allocate(u32 size);
		FORCEINLINE void Push();
		FORCEINLINE void Pop();
		FORCEINLINE MemoryArenaScope PushScope();
		u32 GetUsedBytes() { return mUsedBytes; }
		u8* GetPtr() { return mPtr; }
	private:
		u8* mPtr;
		u32 mUsedBytes;
		u32 mTotalBytes;
		u32 mDepth;
		u32 mOffsets[MaxDepth];
		friend class MemoryArenaScope;
	};

	MemoryArenaScope::MemoryArenaScope(ScratchStack& arena, u32 depth)
		: mArena(arena)
		, mDepth(depth)
	{}
	MemoryArenaScope::~MemoryArenaScope()
	{
		mArena.mDepth = mDepth;
		mArena.mUsedBytes = mArena.mOffsets[mDepth];
	}

	u8* ScratchStack::Allocate(u32 size)
	{
		u8* ret = mPtr + mUsedBytes;
		mUsedBytes += AlignUpSize(size, Alignment);
		CheckDebug(mUsedBytes <= mTotalBytes);
		return ret;
	}
	void ScratchStack::Push()
	{
		CheckDebug(mDepth < MaxDepth);
		mOffsets[mDepth++] = mUsedBytes;
	}
	void ScratchStack::Pop()
	{
		CheckDebug(mDepth > 0);
		mUsedBytes = mOffsets[--mDepth];
	}
	MemoryArenaScope ScratchStack::PushScope()
	{
		Push();
		return MemoryArenaScope(*this, mDepth - 1);
	}

}
