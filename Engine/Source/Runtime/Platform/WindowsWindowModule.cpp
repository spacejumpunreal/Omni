#include "Runtime/Platform/WindowModule.h"
#include "Runtime/Memory/MemoryModule.h"
#include "Runtime/Misc/PImplUtils.h"
#include "Runtime/System/ModuleExport.h"
#include "Runtime/Test/AssertUtils.h"

#if OMNI_WINDOWS

namespace Omni
{
    //declarations
    struct WindowsWindowModulePrivate
    {
    public:
    public:
    };

    using WindowsWindowModuleImpl = PImplCombine<WindowModule, WindowsWindowModulePrivate>;

    //globals
    WindowsWindowModuleImpl* gWindowsWindowModule;

    //implementations
    void WindowModule::Initialize()
    {
        MemoryModule::Get().Retain();
        WindowsWindowModuleImpl* self = WindowsWindowModuleImpl::GetCombinePtr(this);
        CheckAlways(gWindowsWindowModule == nullptr);
        gWindowsWindowModule = self;

        //here

        Module::Initialize();
    }
    void WindowModule::Finalize()
    {
        Module::Finalizing();
        if (GetUserCount() > 0)
            return;

        //here
    }
    void WindowModule::Finalizing()
    {
        Finalize();
    }
    WindowModule& WindowModule::Get()
    {
        return *gWindowsWindowModule;
    }
}
#endif
