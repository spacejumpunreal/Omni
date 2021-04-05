#pragma once
#include "Runtime/Omni.h"

namespace Omni
{
	class Module;
	enum class SystemStatus : u8
	{
		Uninitialized,
		Initializing,
		Ready,
		ToBeFinalized,
		Finalizing,
	};

	using ModuleKey = i32;
	using SystemInitializedCallback = void(*)();

	class System
	{
	public:
		//APIs for Engine owenr
		static void CreateSystem();
		static System& GetSystem();
		void DestroySystem();

		//EngineModule MainThread
		//whoever called this became MainThread
		void InitializeAndJoin(u32 argc, const char** argv, SystemInitializedCallback onSystemInitialized); 
		void Finalize();

		//API for Engine users
		SystemStatus GetStatus();
		void TriggerFinalization(bool assertOnMiss);

		//APIs for Modules
		Module* GetModule(ModuleKey moduleKey) const;
		Module* GetModule(const char* s) const;
	};
}
