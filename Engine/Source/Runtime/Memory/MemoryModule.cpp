#include "Runtime/Memory/MemoryModule.h"
#include "Runtime/Misc/AssertUtils.h"
#include "Runtime/Misc/PImplUtils.h"
#include "Runtime/Misc/ThreadLocalData.h"
#include "Runtime/Memory/MemoryArena.h"
#include "Runtime/Memory/SNAllocator.h"
#include "Runtime/Memory/WrapperAllocator.h"
#include "Runtime/System/Module.h"
#include "Runtime/System/ModuleExport.h"


namespace Omni
{
	//flags
	static constexpr bool StatsMemoryKinds = false;

	//forard decalrations
	class IAllocator;

	//definitions
	struct MemoryModulePrivateData
	{
		static constexpr size_t			DefaultThreadArenaSize = 1024 * 1024;

		PMRResource*					mKind2PMRResources[(size_t)MemoryKind::Max];
		IAllocator*						mAllocators[(size_t)MemoryKind::Max];
		u32								mThreadArenaSize;

		MemoryModulePrivateData()
			: mThreadArenaSize(DefaultThreadArenaSize)
		{
			memset(mKind2PMRResources, 0, sizeof(mKind2PMRResources));
			memset(mAllocators, 0, sizeof(mAllocators));
		}
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
		MemoryModulePrivateData* self = MemoryModuleImpl::GetCombinePtr(this);
		size_t usedAllocators = 0;
		IAllocator* primary = new SNAllocator();
		self->mAllocators[usedAllocators++] = primary;
		self->mKind2PMRResources[(u32)MemoryKind::SystemInit] = primary->GetResource();

		if constexpr (StatsMemoryKinds)
		{
			const char* MemoryKindNames[] = 
			{
#define MEMORY_KIND(X) #X,
#include "Runtime/Memory/MemoryKind.inl"
#undef MEMORY_KIND
			};
			for (u32 iKind = (u32)MemoryKind::SystemInit + 1; iKind < (u32)MemoryKind::Max; ++iKind)
			{
				IAllocator* alloc = self->mAllocators[usedAllocators++] = new WrapperAllocator(*primary->GetResource(), MemoryKindNames[iKind]);
				self->mKind2PMRResources[iKind] = alloc->GetResource();
			}
		}
		else
		{
			for (u32 iKind = (u32)MemoryKind::SystemInit + 1; iKind < (u32)MemoryKind::Max; ++iKind)
			{
				self->mKind2PMRResources[iKind] = primary->GetResource();
			}
		}

		gMemoryModule = this;
		Module::Initialize();
	}
	void MemoryModule::Finalize()
	{
		MemoryModulePrivateData* self = MemoryModuleImpl::GetCombinePtr(this);
		Module::Finalizing();
		if (GetUserCount() > 0)
			return;

		for (i32 iAllocator = (i32)MemoryKind::Max - 1; iAllocator >= 0; --iAllocator)
		{
			IAllocator* alloc = self->mAllocators[iAllocator];
			if (alloc == nullptr)
				continue;
			MemoryStats ms = alloc->GetStats();
			CheckAlways(ms.Used == 0);
			delete alloc;
		}
		Module::Finalize();
		gMemoryModule = nullptr;
	}
	MemoryModule::~MemoryModule()
	{
	}
	MemoryModule& MemoryModule::Get()
	{
		return *gMemoryModule;
	}
	PMRAllocator MemoryModule::GetPMRAllocator(MemoryKind kind)
	{
		MemoryModuleImpl* self = MemoryModuleImpl::GetCombinePtr(this);
		return self->mKind2PMRResources[(u32)kind];
	}
	MemoryArena& MemoryModule::GetThreadArena()
	{
		return gThreadArena;
	}
	void MemoryModule::ThreadInitialize()
	{
		CheckAlways(gThreadArena.GetPtr() == nullptr);
		MemoryModuleImpl* self = MemoryModuleImpl::GetCombinePtr(gMemoryModule);
		void* p = gMemoryModule->GetPMRAllocator(MemoryKind::ThreadArena).resource()->allocate(self->mThreadArenaSize, OMNI_DEFAULT_ALIGNMENT);
		gThreadArena.Reset((u8*)p, self->mThreadArenaSize);
	}
	void MemoryModule::ThreadFinalize()
	{
		CheckAlways(gThreadArena.GetPtr() != nullptr);
		MemoryModuleImpl* self = MemoryModuleImpl::GetCombinePtr(gMemoryModule);
		void* p = gThreadArena.GetPtr();
		gMemoryModule->GetPMRAllocator(MemoryKind::ThreadArena).resource()->deallocate(p, self->mThreadArenaSize, OMNI_DEFAULT_ALIGNMENT);
		gThreadArena.Reset(nullptr, 0);
	}
	void MemoryModule::GetStats(std::pmr::vector<MemoryStats>& ret)
	{
		MemoryModuleImpl* self = MemoryModuleImpl::GetCombinePtr(this);
		for (IAllocator* a : self->mAllocators)
		{
			ret.push_back(a->GetStats());
		}
	}
	void MemoryModule::Shrink()
	{}
	Module* MemoryModuleCtor(const EngineInitArgMap&)
	{
		return new MemoryModuleImpl();
	}
	ExportInternalModule(Memory, ModuleExportInfo(MemoryModuleCtor, true));
}