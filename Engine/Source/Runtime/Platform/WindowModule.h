#pragma once
#include "Runtime/Omni.h"
#include "Runtime/System/Module.h"


namespace Omni
{
	class WindowModule : public Module
	{
	public:
		void Initialize(const EngineInitArgMap&) override;
		void Finalize() override;
		void Finalizing() override;
		static WindowModule& Get();

		void GetBackbufferSize(u32& width, u32& height);
		void SetBackbufferSize(u32 width, u32 height);
	};
}

