#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Memory/MemoryDefs.h"

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