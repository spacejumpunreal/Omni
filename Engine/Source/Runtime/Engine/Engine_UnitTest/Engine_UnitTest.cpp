#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Core/System/System.h"
#include "gtest/gtest.h"

namespace Omni
{
	void EngineUnitTestCode()
	{}
}

int main(int argc, char** argv) {
	printf("Running main() from %s\n", __FILE__);


	Omni::System::CreateSystem();
	Omni::System& system = Omni::System::GetSystem();
	system.InitializeAndJoin(0, nullptr, Omni::EngineUnitTestCode, nullptr);
	system.DestroySystem();


	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
