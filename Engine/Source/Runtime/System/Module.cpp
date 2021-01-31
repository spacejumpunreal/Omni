#include "Runtime/System/Module.h"
#include "Runtime/Misc/AssertUtils.h"
namespace Omni
{
	void Module::Initialize() 
	{
		CheckAlways(mStatus == ModuleStatus::Uninitialized);
		mStatus = ModuleStatus::Ready;
	};
	void Module::Initializing() 
	{};
	void Module::Finalize() 
	{
		mStatus = ModuleStatus::Uninitialized;
	};
	void Module::Finalizing() 
	{};
	ModuleStatus Module::GetStatus() //supposed to be called on MainThread during Initialization and Finalization
	{
		return mStatus;
	}
	Module::~Module() 
	{};
}