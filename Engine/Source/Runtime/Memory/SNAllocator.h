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
		~SNAllocator();
		PMRResource* GetResource() override;
		MemoryStats GetStats() override;
		const char* GetName() override;
		void Shrink() override;
	private:
		PrivateData<48> mData;
	};
}