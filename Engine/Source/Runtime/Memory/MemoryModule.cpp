#include "Runtime/Memory/MemoryModule.h"

#include "Runtime/Memory/SNMallocWrapper.h"
#include "Runtime/Misc/PImplUtils.h"
#include "Runtime/System/Module.h"
#include "Runtime/System/ModuleExport.h"

namespace Omni
{
	//forard decalrations
	struct MemoryModulePrivateData
	{
	};

	//definitions
	using MemoryModuleImpl = PImplCombine<MemoryModule, MemoryModulePrivateData>;

	void MemoryModule::Initialize()
	{
	}

	void MemoryModule::Initializing()
	{
	}

	void MemoryModule::Finalize()
	{
	}

	void MemoryModule::Finalizing()
	{
	}

	void MemoryModule::Destroy()
	{
	}

	ModuleStatus MemoryModule::GetState()
	{
		return ModuleStatus();
	}

	Module* MemoryModuleCtor(const EngineInitArgMap&)
	{
		return new MemoryModuleImpl();
	}
	ExportInternalModule(Memory, ModuleExportInfo(MemoryModuleCtor, true));
}