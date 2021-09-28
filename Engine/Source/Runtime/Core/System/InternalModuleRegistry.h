#pragma once
#include "Omni.h"

namespace Omni
{
	class Module;
	class System;

#define ModuleItem(ModuleName) ModuleName,
	enum class EngineModuleKeys : u8
	{
		#include "System/InternalModuleRegistry.inl"
		EngineModuleCount,
	};
#undef ModuleItem
}