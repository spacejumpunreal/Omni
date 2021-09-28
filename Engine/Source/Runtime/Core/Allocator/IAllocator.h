#pragma once
#include "Omni.h"
#include "Memory/MemoryDefs.h"

namespace Omni
{
	class IAllocator
	{
	public:
		virtual PMRResource* GetResource() = 0;
		virtual MemoryStats GetStats() = 0;
		virtual const char* GetName() = 0;
		virtual void Shrink() = 0;
		virtual ~IAllocator() {};
	};
}

