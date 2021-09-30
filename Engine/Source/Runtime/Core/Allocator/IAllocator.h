#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Memory/MemoryDefs.h"

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

