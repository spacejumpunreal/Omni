#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/System/System.h"

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

	/*
	* # init sequence
	* - ctor called for module
	* - call Initialize() until if Module::Initialize() is called by this module(considered fully initialized)
	* - Retain() may be called by others to declare reference
	*
	* # destroy sequence
	* - Release() called until eached zero reference
	* - call Finalize() until if Module::Finalize() is called by this module(considered that finalize is done)
	* - call dtor to free memory
	* 
	*/

	class CORE_API Module
	{
	public:
		Module();
		virtual void Destroy() = 0;
		virtual void Initialize(const EngineInitArgMap&);
		void Initializing();
		virtual void Finalize();
		void Finalizing();
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
