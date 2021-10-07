#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"

namespace Omni
{
	class MonotonicMemoryResource;
	class Module;
	struct ModuleExportInfo;

	using ModuleKey = i32;
	using SystemInitializedCallback = void(*)();

	enum class SystemStatus : u8
	{
		Uninitialized,
		Initializing,
		Ready,
		ToBeFinalized,
		Finalizing,
	};


	class CORE_API System
	{
	public:
		//APIs for Engine user
		static void CreateSystem();
		static System& GetSystem();
		void DestroySystem();
		////the thread that InitializeAndJoin this beomes MainThread
		void InitializeAndJoin(u32 argc, const char** argv, SystemInitializedCallback onSystemInitialized);
		SystemStatus GetStatus();
		void TriggerFinalization(bool assertOnMiss);

		//APIs for Modules
		void RegisterModule(const ModuleExportInfo& moduleInfo);
		Module* GetModule(ModuleKey moduleKey) const;
		Module* GetModule(const char* s) const;
		MonotonicMemoryResource& GetInitMemResource();

		//internal use
		void Finalize();
	};
}
