#pragma once
#include "Runtime/Omni.h"
#include <memory_resource>

namespace Omni
{
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
		u32 GetUsedBytes() { return mUsedBytes; }
	private:
		u8* mPtr;
		u32 mUsedBytes;
		u32 mTotalBytes;
		u32 mDepth;
		u32 mOffsets[MaxDepth];
	};
}
