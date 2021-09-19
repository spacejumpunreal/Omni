#pragma once
#include "Runtime/Omni.h"
#include "Runtime/System/System.h"

namespace Omni
{
	class Module;
	class System;

	class EngineInitArgMap;
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
		virtual void Destroy() = 0;
		virtual void Initialize(const EngineInitArgMap&);
		virtual void Initializing();
		virtual void Finalize();
		virtual void Finalizing();
		virtual ~Module();

		ModuleStatus GetStatus(); //supposed to be called on MainThread during Initialization and Finalization
		void Retain();
		void Release();
	protected:
		u32 GetUserCount();
	private:
		ModuleStatus	mStatus;
		u32				mUserCount;
	};
}
