#pragma once
#include "Runtime/Omni.h"
#include "Runtime/System/Module.h"

namespace Omni
{
	class ConcurrencyModule : public Module
	{
	public:
		void Initialize() override;
		void Finalize() override;

		static ConcurrencyModule& Get();
	};
}