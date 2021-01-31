#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Memory/IAllocator.h"
#include "Runtime/Misc/PrivateData.h"

namespace Omni
{
	class SNAllocator final : public IAllocator
	{
	public:
		SNAllocator();
		virtual PMRAllocator GetPMRAllocator();
		virtual MemoryStats GetStats();
		virtual const char* GetName();
		virtual void Shrink();
	private:
		PrivateData<32> mData;
	};
}