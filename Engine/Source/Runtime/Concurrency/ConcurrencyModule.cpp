#include "Runtime/Concurrency/ConcurrencyModule.h"
#include "Runtime/System/ModuleExport.h"
#include "Runtime/System/Module.h"

#include "Runtime/Misc/PImplUtils.h"

namespace Omni
{
	//forard decalrations
	struct ConcurrencyModulePrivateData
	{
	};

	//definitions
	using ConcurrencyModuleImpl = PImplCombine<ConcurrencyModule, ConcurrencyModulePrivateData>;

	void ConcurrencyModule::Initialize()
	{
	}

	void ConcurrencyModule::Initializing()
	{
	}

	void ConcurrencyModule::Finalize()
	{
	}

	void ConcurrencyModule::Finalizing()
	{
	}

	void ConcurrencyModule::Destroy()
	{
	}

	ModuleStatus ConcurrencyModule::GetState()
	{
		return ModuleStatus();
	}

	Module* ConcurrencyModuleCtor(const EngineInitArgMap&)
	{
		return new ConcurrencyModuleImpl();
	}
	ExportInternalModule(Concurrency, ModuleExportInfo(ConcurrencyModuleCtor, true));
}