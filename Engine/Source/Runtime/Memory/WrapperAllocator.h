#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Memory/IAllocator.h"
#include "Runtime/Misc/PrivateData.h"


namespace Omni
{
	class WrapperAllocator : public IAllocator
	{
		static constexpr size_t PrivateDataSize = 40;
	public:
		WrapperAllocator(std::pmr::memory_resource& memResource, const char* name);
		PMRAllocator GetPMRAllocator() override;
		MemoryStats GetStats() override;
		const char* GetName() override;
		void Shrink() override
		{}
	private:
		PrivateData<PrivateDataSize>	mData;
	};
}