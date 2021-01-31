#pragma once
#include "Runtime/Memory/SNAllocator.h"

#pragma warning( push )
#pragma warning( disable : 4324 4127)
#include "External/snmalloc/src/snmalloc.h"
#pragma warning( pop )

namespace Omni
{
	struct SNAllocatorPrivate
	{
	};
	SNAllocator::SNAllocator()
		: mData(PrivateDataType<SNAllocatorPrivate>{})
	{}
	PMRAllocator SNAllocator::GetPMRAllocator()
	{
		return PMRAllocator();
	}
	MemoryStats SNAllocator::GetStats()
	{
		return MemoryStats();
	}
	const char* SNAllocator::GetName()
	{
		return nullptr;
	}
	void SNAllocator::Shrink()
	{
	}
}