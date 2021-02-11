#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Memory/IAllocator.h"
#include "Runtime/Misc/PrivateData.h"

namespace Omni
{
	class CacheLineAllocator : public IAllocator
	{
	public:
		CacheLineAllocator();
		~CacheLineAllocator();
		PMRResource* GetResource() override;
		MemoryStats GetStats() override;
		const char* GetName() override;
		void Shrink() override;
	private:
		PrivateData<168> mData;
	};
}