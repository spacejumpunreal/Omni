#pragma once

#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include <map>
#include <string>

namespace Omni
{
	class Module;
	class EngineInitArgMap : public std::multimap<std::string, std::string> {};
	using ModuleCtor = Module* (*)(const EngineInitArgMap&);
	struct ModuleExportInfo
	{
		ModuleExportInfo(ModuleCtor ctor, bool isAlwaysLoad, const char* name=nullptr, i32 key=-1)
			: Ctor(ctor)
			, Name(name)
			, Key(key)
			, IsAlwaysLoad(isAlwaysLoad)
		{}
		ModuleCtor		Ctor;
		const char*		Name;
		i32				Key;
		bool			IsAlwaysLoad;
	};
}

#define ModuleCreationStubName(moduleName) moduleName##ExportInfoStub
#define ExportInternalModule(moduleName, info) ModuleExportInfo ModuleCreationStubName(moduleName) = info;

