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
		ModuleExportInfo(const ModuleExportInfo& other) = default;
		ModuleCtor		Ctor;
		const char*		Name;
		i32				Key;
		bool			IsAlwaysLoad;
	};
}

#define MODULE_CREATION_STUB_NAME(moduleName) moduleName##ExportInfoStub
#define EXPORT_INTERNAL_MODULE(moduleName, info) ModuleExportInfo MODULE_CREATION_STUB_NAME(moduleName) = info;

