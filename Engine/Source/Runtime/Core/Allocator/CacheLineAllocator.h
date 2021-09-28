#pragma once
#include "Omni.h"
#include "Allocator/IAllocator.h"
#include "Misc/PrivateData.h"
#include "PlatformDefs.h"
#include "SuppressWarning.h"

namespace Omni
{
	struct CacheLinePerThreadData;

	OMNI_PUSH_WARNING()
	OMNI_SUPPRESS_WARNING_PADDED_DUE_TO_ALIGNMENT()
	class CacheLineAllocator : public IAllocator
	{
	public:
		CacheLineAllocator();
		~CacheLineAllocator();
		PMRResource* GetResource() override;
		MemoryStats GetStats() override;
		const char* GetName() override;
		void Shrink() override;
		void ThreadInitialize();
		void ThreadFinalize();
	private:
		PrivateData<256, CPU_CACHE_LINE_SIZE> mData;
	};
	OMNI_POP_WARNING()
}

