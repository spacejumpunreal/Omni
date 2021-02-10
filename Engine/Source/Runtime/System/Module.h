#pragma once
#include "Runtime/Omni.h"
#include "Runtime/System/System.h"

namespace Omni
{
	class Module;
	class System;

	enum class ModuleStatus : u8
	{
		Uninitialized,
		Initializing,
		Ready,
		Finalizing,
	};

	class Module
	{
	public:
		Module();
		virtual void Initialize();
		virtual void Initializing();
		virtual void Finalize();
		virtual void Finalizing();
		virtual ~Module();

		ModuleStatus GetStatus(); //supposed to be called on MainThread during Initialization and Finalization
		void Acquire();
		void Release();
	protected:
		u32 GetUserCount();
	private:
		ModuleStatus	mStatus;
		u32				mUserCount;
	};
}
