#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Base/Misc/PrivateData.h"
#include "Runtime/Core/Allocator/IAllocator.h"
#include "Runtime/Prelude/PlatformDefs.h"
#include "Runtime/Prelude/SuppressWarning.h"

namespace Omni
{
	struct CacheLinePerThreadData;

	class CacheLineAllocator : public IAllocator
	{
	public:
		static CacheLineAllocator* Create();
		void Destroy() override;
		PMRResource* GetResource() override;
		MemoryStats GetStats() override;
		const char* GetName() override;
		void Shrink() override;
		void ThreadInitialize();
		void ThreadFinalize();
	};
}

