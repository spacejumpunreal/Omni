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

	//global variables
	ConcurrencyModule* gConcurrencyModule;

	void ConcurrencyModule::Initialize()
	{
		gConcurrencyModule = this;
		Module::Initialize();
	}
	void ConcurrencyModule::Finalize()
	{
		Module::Finalizing();
		if (GetUserCount() > 0)
			return;

		gConcurrencyModule = nullptr;
		Module::Finalize();
	}
	ConcurrencyModule& ConcurrencyModule::Get()
	{
		return *gConcurrencyModule;
	}
	Module* ConcurrencyModuleCtor(const EngineInitArgMap&)
	{
		return new ConcurrencyModuleImpl();
	}
	ExportInternalModule(Concurrency, ModuleExportInfo(ConcurrencyModuleCtor, true));
}