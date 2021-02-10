#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Memory/MemoryArena.h"
#include "Runtime/Memory/MemoryDefs.h"
#include "Runtime/Memory/MemoryWatch.h"
#include "Runtime/System/Module.h"

namespace Omni
{
	class MemoryArena;

	class MemoryModule : public Module
	{
	public:
		~MemoryModule();
		void Initialize() override;
		void Finalize() override;

		static MemoryModule& Get();
		PMRAllocator GetPMRAllocator(MemoryKind kind);
		static FORCEINLINE MemoryArena& GetThreadArena();
		void ThreadInitialize();
		void ThreadFinalize();
		void GetStats(std::pmr::vector<MemoryStats>& stats);
		void Shrink();
	};
}