#include "Runtime/System/System.h"
#include "Runtime/Misc/ArrayUtils.h"
#include <array>

extern "C"
{
    int IOSMain(int argc, const char** argv);
}

int main(int argc, const char** argv)
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
    
    return IOSMain(argc, argv);
}
