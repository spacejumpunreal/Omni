#pragma once
#include "Runtime/Omni.h"

namespace Omni
{
	class Module;
	class System;

#define ModuleItem(ModuleName) ModuleName,
	enum class EngineModuleKeys : u8
	{
		#include "Runtime/System/InternalModuleRegistry.inl"
		EngineModuleCount,
	};
#undef ModuleItem
}