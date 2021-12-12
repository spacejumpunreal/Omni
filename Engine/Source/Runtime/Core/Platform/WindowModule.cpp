#include "Runtime/Core/CorePCH.h"
#include "Runtime/Core/Platform/WindowModule.h"

namespace Omni
{
    //globals
    WindowModule* WindowModule::gWindowModule;

    WindowModule& WindowModule::Get()
    {
        return *gWindowModule;
    }
    WindowModule* WindowModule::GetPtr()
    {
        return gWindowModule;
    }

}