#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/Allocator/IAllocator.h"
#include "Runtime/Base/Misc/PrivateData.h"


namespace Omni
{
	class WrapperAllocator : public IAllocator
	{
	public:
		WrapperAllocator(StdPmr::memory_resource& memResource, const char* name);
		~WrapperAllocator();
		PMRResource* GetResource() override;
		MemoryStats GetStats() override;
		const char* GetName() override;
		void Shrink() override;
	private:
		PrivateData<56>	mData;
	};
}

