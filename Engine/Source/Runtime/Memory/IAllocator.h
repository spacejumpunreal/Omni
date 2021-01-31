#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Memory/MemoryModule.h"

namespace Omni
{
	class IAllocator
	{
	public:
		virtual PMRAllocator GetPMRAllocator() = 0;
		virtual MemoryStats GetStats() = 0;
		virtual const char* GetName() = 0;
		virtual void Shrink() = 0;
	};
}