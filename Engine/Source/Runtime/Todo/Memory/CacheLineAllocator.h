#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Memory/IAllocator.h"
#include "Runtime/Misc/PrivateData.h"

namespace Omni
{
	struct CacheLinePerThreadData;

	OMNI_MSVC_DISABLE_WARNING(4324);
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
		PrivateData<256, 128> mData;
	};
	OMNI_RESET_WARNING();
}

