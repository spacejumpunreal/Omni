#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/Allocator/IAllocator.h"
#include "Runtime/Base/Misc/PrivateData.h"

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

