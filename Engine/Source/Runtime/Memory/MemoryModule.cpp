#include "Runtime/Memory/MemoryModule.h"

#include "Runtime/Memory/SNMallocWrapper.h"
#include "Runtime/Misc/AssertUtils.h"
#include "Runtime/Misc/PImplUtils.h"
#include "Runtime/Misc/ThreadLocalData.h"
#include "Runtime/System/Module.h"
#include "Runtime/System/ModuleExport.h"


namespace Omni
{
	//forard decalrations
	class IAllocator;

	//definitions
	struct MemoryModulePrivateData
	{
		PMRAllocator					mKind2PMRAllocators[(size_t)MemoryKind::Max];
		IAllocator**					mAllocators;
		u32								mThreadArenaSize;
	};
	using MemoryModuleImpl = PImplCombine<MemoryModule, MemoryModulePrivateData>;

	//globals
	MemoryModule*						gMemoryModule;
	thread_local static MemoryArena		gThreadArena;
	//methods
	void MemoryModule::Initialize()
	{
		CheckAlways(gMemoryModule == nullptr, "singleton rule violated");
		CheckAlways(gThreadArena.GetUsedBytes() == 0);
		

		gMemoryModule = this;
	}

	void MemoryModule::Finalize()
	{
		gMemoryModule = nullptr;
	}

	MemoryModule::~MemoryModule()
	{
	}

	MemoryModule& MemoryModule::Get()
	{
		return *gMemoryModule;
	}
	PMRAllocator MemoryModule::GetAllocator(MemoryKind kind)
	{
		MemoryModuleImpl* self = MemoryModuleImpl::GetCombinePtr(this);
		return self->mKind2PMRAllocators[(u32)kind];
	}
	FORCEINLINE MemoryArena& MemoryModule::GetThreadArena()
	{
		return gThreadArena;
	}
	void MemoryModule::ThreadInit()
	{
		MemoryModuleImpl* self = MemoryModuleImpl::GetCombinePtr(this);
		void* p = GetAllocator(MemoryKind::ThreadArena).resource()->allocate(self->mThreadArenaSize, OMNI_DEFAULT_ALIGNMENT);
		gThreadArena.Reset((u8*)p, self->mThreadArenaSize);
	}
	void MemoryModule::GetStats(std::pmr::vector<MemoryStats>&)
	{}
	void MemoryModule::Shrink()
	{}

	Module* MemoryModuleCtor(const EngineInitArgMap&)
	{
		return new MemoryModuleImpl();
	}
	ExportInternalModule(Memory, ModuleExportInfo(MemoryModuleCtor, true));
}