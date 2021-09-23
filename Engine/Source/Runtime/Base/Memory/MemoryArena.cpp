#include "BasePCH.h"
#include "Memory/MemoryArena.h"
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
	u8* ScratchStack::Cleanup()
	{
		CheckAlways(mUsedBytes == 0 && mDepth == 0);
		u8* ret = mPtr;
		mPtr = nullptr;
		mTotalBytes = 0;
		return ret;
	}
	bool ScratchStack::IsClean()
	{
		return mPtr == nullptr && mUsedBytes == 0 && mTotalBytes == 0 && mDepth == 0;
	}
}
