#pragma once
#include "Runtime/Omni.h"
#include "Runtime/System/Module.h"

namespace Omni
{
	class MemoryModule : public Module
	{
	public:
		void Initialize();
		void Initializing();
		void Finalize();
		void Finalizing();
		void Destroy();
		ModuleStatus GetState();
	};
}