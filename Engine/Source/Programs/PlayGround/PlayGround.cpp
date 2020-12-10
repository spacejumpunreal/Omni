#include "Runtime/System/System.h"
#include "Runtime/Misc/ArrayUtils.h"
#include <array>

int main(int, const char**)
{
	Omni::System::CreateSystem();
	Omni::System& system = Omni::System::GetSystem();
	const char* argv[] = 
	{
		"",
	};
	system.InitializeAndJoin(ARRAY_LENGTH(argv), argv);
	system.TriggerFinalization();
	system.WaitTillFinalized();
	system.DestroySystem();
}