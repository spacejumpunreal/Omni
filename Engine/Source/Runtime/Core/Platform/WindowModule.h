#pragma once
#include "Omni.h"
#include "System/Module.h"


namespace Omni
{
	class WindowModule : public Module
	{
	public:
		void Destroy() override;
		void Initialize(const EngineInitArgMap&) override;
		void Finalize() override;
		void Finalizing() override;
		static WindowModule& Get();
		static WindowModule* GetPtr();

		void RunUILoop();
		void GetBackbufferSize(u32& width, u32& height);
		void RequestSetBackbufferSize(u32 width, u32 height);
	};
}

