#include "Runtime/System/System.h"
#include "Runtime/Misc/ArrayUtils.h"


int main(int, const char** )
{
	Omni::System::CreateSystem();
	Omni::System& system = Omni::System::GetSystem();
	const char* engineArgv[] =
	{
		"",
	};
	system.InitializeAndJoin(ARRAY_LENGTH(engineArgv), engineArgv);
	system.TriggerFinalization();
	system.WaitTillFinalized();
	system.DestroySystem();
 
	return 0;
}
