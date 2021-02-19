#include "Runtime/Omni.h"
#include "Runtime/Memory/MemoryArena.h"
#include <cstring>
namespace Omni
{
	ScratchStack::ScratchStack()
		: mPtr(nullptr)
		, mUsedBytes(0)
		, mTotalBytes(0)
		, mDepth(0)
	{
		memset(mOffsets, 0, sizeof(mOffsets));
	}
	void ScratchStack::Reset(u8* ptr, u32 bytes)
	{
		CheckAlways(IsAligned(ptr, Alignment));
		CheckAlways(mUsedBytes == 0 && mDepth == 0);
		mPtr = ptr;
		mTotalBytes = bytes;
	}
	bool ScratchStack::IsClean()
	{
		return mPtr == nullptr && mUsedBytes == 0 && mTotalBytes == 0 && mDepth == 0;
	}
}
