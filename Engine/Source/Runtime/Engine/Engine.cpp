#include "Runtime/Engine/EnginePCH.h"
#include "Runtime/Engine/Engine.h"
#include "Runtime/Base/Misc/ArrayUtils.h"
#include "Runtime/Core/System/ModuleExport.h"
#include "Runtime/Core/System/System.h"

namespace Omni
{

#define EngineModuleItem(name) extern ModuleExportInfo MODULE_CREATION_STUB_NAME(name);
#include "Runtime/Engine/EngineModuleRegistry.inl"
#undef EngineModuleItem


#define EngineModuleItem(name) &MODULE_CREATION_STUB_NAME(name),
    static const ModuleExportInfo* EnginelModuleInfo[] = {
#include "Runtime/Engine/EngineModuleRegistry.inl"
    };
#undef EngineModuleItem

    System& CreateEngineSystem()
    {
        System::CreateSystem();
        System& system = System::GetSystem();
        for (size_t i = 0; i < ARRAY_LENGTH(EnginelModuleInfo); ++i)
        {
            system.RegisterModule(*EnginelModuleInfo[i]);
        }
        return system;
    }

}