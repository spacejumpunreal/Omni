#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"

namespace Omni
{
	class Module;
	class System;

#define ModuleItem(ModuleName) ModuleName,
	enum class EngineModuleKeys : u8
	{
#include "Runtime/Core/System/InternalModuleRegistry.inl"
		EngineModuleCount,
	};
#undef ModuleItem
}
