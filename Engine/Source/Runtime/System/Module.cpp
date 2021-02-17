#include "Runtime/System/Module.h"
#include "Runtime/Test/AssertUtils.h"
namespace Omni
{
	Module::Module()
		: mStatus(ModuleStatus::Uninitialized)
		, mUserCount(0)
	{}
	void Module::Initialize() 
	{
		mStatus = ModuleStatus::Ready;
	};
	void Module::Initializing() 
	{
		mStatus = ModuleStatus::Initializing;
	};
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