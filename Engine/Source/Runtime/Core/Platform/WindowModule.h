#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/System/Module.h"
#include "Runtime/Core/Platform/WindowUtils.h"


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

		void GetClientAreaSize(u32& width, u32& height);
		void RequestSetClientAreaSize(u32 width, u32 height);
		WindowHandle GetMainWindowHandle();

	protected:
		static WindowModule* gWindowModule;
	};
}

