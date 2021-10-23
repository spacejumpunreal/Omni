#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/System/Module.h"
#include "Runtime/Base/Misc/AssertUtils.h"

namespace Omni
{
	Module::Module()
		: mStatus(ModuleStatus::Uninitialized)
		, mUserCount(0)
	{}
	void Module::Initialize(const EngineInitArgMap&)
	{
		mStatus = ModuleStatus::Ready;
	};
	void Module::Initializing() 
	{
		mStatus = ModuleStatus::Initializing;
	};
	void Module::StopThreads()
	{}
	void Module::Finalize() 
	{
		mStatus = ModuleStatus::Uninitialized;
	};
	void Module::Finalizing() 
	{
		mStatus = ModuleStatus::Finalizing;
	};
	ModuleStatus Module::GetStatus() //supposed to be called on MainThread during Initialization and Finalization
	{
		return mStatus;
	}
	void Module::Retain()
	{
		++mUserCount;
	}
	void Module::Release()
	{
		--mUserCount;
	}
	u32 Module::GetUserCount()
	{
		return mUserCount;
	}
	Module::~Module() 
	{
		CheckAlways(mUserCount == 0);
	};
}

