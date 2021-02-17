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

		template<typename T>
		PMRAllocatorT<T> GetPMRAllocatorT(MemoryKind kind)
		{
			return PMRAllocatorT<T>(GetPMRAllocator(kind));
		}

		static ScratchStack& GetThreadScratchStack();
		static void ThreadInitialize();
		static void ThreadFinalize();
		void GetStats(std::pmr::vector<MemoryStats>& stats);
		void Shrink();
	};

	template<typename T>
	void DeleteForType(T* p, MemoryKind kind, std::align_val_t align = {})
	{
		p->~T();
		MemoryModule::Get().GetPMRAllocator(kind).resource()->deallocate(
			p, 
			sizeof(T), 
			align == std::align_val_t {} ? OMNI_DEFAULT_ALIGNMENT : (size_t)align);
	}
}

FORCEINLINE void* operator new(size_t size, Omni::MemoryKind kind)
{
	return Omni::MemoryModule::Get().GetPMRAllocator(kind).resource()->allocate(size, OMNI_DEFAULT_ALIGNMENT);
}
FORCEINLINE void* operator new(size_t size, std::align_val_t align, Omni::MemoryKind kind)
{
	return Omni::MemoryModule::Get().GetPMRAllocator(kind).resource()->allocate(size, (size_t)align);
}
void operator delete(void*, Omni::MemoryKind);//only when there's exception in ctor

#define OMNI_NEW(Kind) new (Kind)
#define OMNI_DELETE(Ptr, Kind) DeleteForType(Ptr, Kind); Ptr = nullptr