#pragma once
#include "Runtime/Omni.h"
#include "Runtime/System/System.h"

namespace Omni
{
	class Module;
	class System;

	enum class ModuleStatus : u8
	{
		Uninitialized,
		Initializing,
		Ready,
		Finalizing,
	};

	class Module
	{
	public:
		virtual void Initialize() {};
		virtual void Initializing() {};
		virtual void Finalize() {};
		virtual void Finalizing() {};
		virtual void Destroy() = 0;
		virtual ModuleStatus GetState() //supposed to be called on MainThread during Initialization and Finalization
		{
			return ModuleStatus::Ready;
		}
	};
}
