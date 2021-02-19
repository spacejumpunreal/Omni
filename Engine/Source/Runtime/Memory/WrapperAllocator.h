#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Memory/IAllocator.h"
#include "Runtime/Misc/PrivateData.h"


namespace Omni
{
	class WrapperAllocator : public IAllocator
	{
	public:
		WrapperAllocator(STD_PMR_NS::memory_resource& memResource, const char* name);
		~WrapperAllocator();
		PMRResource* GetResource() override;
		MemoryStats GetStats() override;
		const char* GetName() override;
		void Shrink() override;
	private:
		PrivateData<56>	mData;
	};
}