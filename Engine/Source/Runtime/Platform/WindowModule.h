#pragma once
#include "Runtime/Omni.h"
#include "Runtime/System/Module.h"


namespace Omni
{
	class WindowModule : public Module
	{
	public:
		void Initialize() override;
		void Finalize() override;
		void Finalizing() override;
		static WindowModule& Get();
	};
}

