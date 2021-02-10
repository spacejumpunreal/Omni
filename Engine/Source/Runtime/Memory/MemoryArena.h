#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Misc/ArrayUtils.h"
#include "Runtime/Misc/AssertUtils.h"
#include <memory_resource>

namespace Omni
{
	class MemoryArena;

	class MemoryArenaScope
	{
	private:
		FORCEINLINE MemoryArenaScope(MemoryArena& arena, u32 depth);
	public:
		FORCEINLINE ~MemoryArenaScope();
	private:
		MemoryArena& mArena;
		u32 mDepth;
		friend class MemoryArena;
	};
	class MemoryArena
	{
	public:
		static const u32 MaxDepth = 16;
		static const u32 Alignment = OMNI_DEFAULT_ALIGNMENT;
	public:
		MemoryArena();
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

	MemoryArenaScope::MemoryArenaScope(MemoryArena& arena, u32 depth)
		: mArena(arena)
		, mDepth(depth)
	{}
	MemoryArenaScope::~MemoryArenaScope()
	{
		mArena.mDepth = mDepth;
		mArena.mUsedBytes = mArena.mOffsets[mDepth];
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
	MemoryArenaScope MemoryArena::PushScope()
	{
		Push();
		return MemoryArenaScope(*this, mDepth - 1);
	}

}
