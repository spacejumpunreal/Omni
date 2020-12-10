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

	class System
	{
	public:
		//APIs for Engine owenr
		static void CreateSystem();
		static System& GetSystem();
		void DestroySystem();
		void InitializeAndJoin(u32 argc, const char** argv); //whoever called this became MainThread

		//API for Engine users
		SystemStatus GetState();
		void TriggerFinalization();
		void WaitTillFinalized();

		//APIs for Modules
		Module* GetModule(ModuleKey moduleKey) const;
		Module* GetModule(const char* s) const;
	};
}
