#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Memory/MemoryDefs.h"
#include "Runtime/Memory/MemoryWatch.h"
#include "Runtime/System/Module.h"

namespace Omni
{

	class ScratchStack;
	class MemoryModule : public Module
	{
	public:
		~MemoryModule();
		void Initialize() override;
		void Finalize() override;

		static MemoryModule& Get();
		PMRAllocator GetPMRAllocator(MemoryKind kind);
		static ScratchStack& GetThreadScratchStack();
		static void ThreadInitialize();
		static void ThreadFinalize();
		void GetStats(std::pmr::vector<MemoryStats>& stats);
		void Shrink();
	};

	template<typename T, MemoryKind kind>
	FORCEINLINE T* AllocForType(size_t count = 1)
	{
		PMRAllocatorT<T> alloc = MemoryModule::Get().GetPMRAllocator(kind);
		return (T*)alloc.allocate(count);
	}

	template<typename T, MemoryKind kind>
	FORCEINLINE void FreeForType(T* p, size_t count = 1)
	{
		PMRAllocatorT<T> alloc = MemoryModule::Get().GetPMRAllocator(kind);
		return alloc.deallocate(p, count);
	}
}