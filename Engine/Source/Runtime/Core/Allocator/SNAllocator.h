#pragma once
#include "Omni.h"
#include "Allocator/IAllocator.h"
#include "Misc/PrivateData.h"

namespace Omni
{
	class SNAllocator final : public IAllocator
	{
	public:
		SNAllocator();
		~SNAllocator();
		PMRResource* GetResource() override;
		MemoryStats GetStats() override;
		const char* GetName() override;
		void Shrink() override;
	private:
		PrivateData<48> mData;
	};
}

