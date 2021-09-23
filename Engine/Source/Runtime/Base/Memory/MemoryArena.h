#pragma once
#include "Omni.h"
#include "PlatformDefs.h"
#include "Misc/ArrayUtils.h"
#include "Misc/AssertUtils.h"

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
		static constexpr u32 MaxDepth = 16;
		static constexpr u32 Alignment = OMNI_DEFAULT_ALIGNMENT;
	public:
		ScratchStack();
		void Reset(u8* ptr, u32 size);
		u8* Cleanup();
		bool IsClean();
		FORCEINLINE u8* Allocate(u32 size);
		FORCEINLINE void Push();
		FORCEINLINE void Pop();
		[[nodiscard]] FORCEINLINE MemoryArenaScope PushScope();
		u32 GetUsedBytes() { return mUsedBytes; }
		u8* GetPtr() { return mPtr; }
	protected:
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
