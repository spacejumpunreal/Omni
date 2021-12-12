#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/System/Module.h"


namespace Omni
{
	class CORE_API WindowModule : public Module
	{
	public:
		void Destroy() override;
		void Initialize(const EngineInitArgMap&) override;
		void StopThreads() override;
		void Finalize() override;
		static WindowModule& Get();
		static WindowModule* GetPtr();

		void GetBackbufferSize(u32& width, u32& height);
		void RequestSetBackbufferSize(u32 width, u32 height);
	protected:
		static WindowModule* gWindowModule;
	};
}

