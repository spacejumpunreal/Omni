#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Base/Memory/MemoryDefs.h"
#include "Runtime/Base/Memory/MemoryWatch.h"
#include "Runtime/Core/System/Module.h"
#include "Runtime/Prelude/PlatformDefs.h"


namespace Omni
{
	class ScratchStack;
	class CORE_API MemoryModule : public Module
	{
	public:
		void Destroy() override;
		void Initialize(const EngineInitArgMap&) override;
		void Finalize() override;

		static MemoryModule& Get();
		PMRAllocator GetPMRAllocator(MemoryKind kind);

		template<typename T>
		PMRAllocatorT<T> GetPMRAllocatorT(MemoryKind kind)
		{
			return PMRAllocatorT<T>(GetPMRAllocator(kind));
		}

		void* Mmap(size_t size, size_t alignment);
		void Munmap(void* mem, size_t size);

		static ScratchStack& GetThreadScratchStack();
		static void ThreadInitialize();
		static void ThreadFinalize();
		void GetStats(StdPmr::vector<MemoryStats>& stats);
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

FORCEINLINE void* OmniMalloc(Omni::MemoryKind kind, size_t size, std::align_val_t align = {})
{
	return Omni::MemoryModule::Get().GetPMRAllocator(kind).resource()->allocate(size, align == std::align_val_t{} ? OMNI_DEFAULT_ALIGNMENT : (size_t)align);
}
FORCEINLINE void OmniFree(Omni::MemoryKind kind, size_t size, void* p, std::align_val_t align = {})
{
	Omni::MemoryModule::Get().GetPMRAllocator(kind).resource()->deallocate(p, size, align == std::align_val_t{} ? OMNI_DEFAULT_ALIGNMENT : (size_t)align);
}

#define OMNI_NEW(Kind) new (Kind)
#define OMNI_DELETE(Ptr, Kind) DeleteForType(Ptr, Kind); Ptr = nullptr
