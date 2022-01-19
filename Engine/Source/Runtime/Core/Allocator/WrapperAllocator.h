
#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Base/Misc/PrivateData.h"
#include "Runtime/Core/Allocator/IAllocator.h"


namespace Omni
{
	class WrapperAllocator : public IAllocator
	{
	public:
		static WrapperAllocator* Create(StdPmr::memory_resource& memResource, const char* name);
		void Destroy() override;
		PMRResource* GetResource() override;
		MemoryStats GetStats() override;
		const char* GetName() override;
		void Shrink() override;
	};
}

