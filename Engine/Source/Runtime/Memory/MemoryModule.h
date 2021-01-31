#pragma once
#include "Runtime/Omni.h"
#include "Runtime/System/Module.h"
#include "Runtime/Memory/MemoryArena.h"

namespace Omni
{
	class MemoryArena;
	using PMRAllocator = std::pmr::polymorphic_allocator<std::byte>;
	
	enum class MemoryKind : u32
	{
		SystemInit,
		ThreadArena,
		Max,
	};

	struct MemoryStats
	{
		u64				Used;
		u64				Reserved;
		const char*		Name;
	};

	class MemoryModule : public Module
	{
	public:
		~MemoryModule();
		void Initialize() override;
		void Finalize() override;

		MemoryModule& Get();
		PMRAllocator GetAllocator(MemoryKind kind);
		static FORCEINLINE MemoryArena& GetThreadArena();
		void ThreadInit();
		void GetStats(std::pmr::vector<MemoryStats>& stats);
		void Shrink();
	};
}