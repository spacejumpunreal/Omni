#include "Runtime/Platform/WindowModule.h"
#if OMNI_WINDOWS
#include "Runtime/Memory/MemoryModule.h"
#include "Runtime/Misc/PImplUtils.h"
#include "Runtime/System/ModuleExport.h"
#include "Runtime/Test/AssertUtils.h"
#include <Windows.h>

namespace Omni
{
    //constants
    static constexpr DWORD OmniMainWindowStyle = WS_OVERLAPPEDWINDOW;
    static constexpr DWORD OmniMainWIndowAdditionalStyle = 0;
    static constexpr u32 DefaultBackbufferWdith = 800;
    static constexpr u32 DefaultBackbufferHeight = 600;

    //declarations
    struct WindowsWindowModulePrivate
    {
    public:
        WindowsWindowModulePrivate(const EngineInitArgMap& args);
        static RECT CalcNonClientWindowRect(u32 topLeftX, u32 topLeftY, u32 width, u32 height);
    public:
        HWND    mWindow;
        u32     mBackbufferWidth;
        u32     mBackbufferHeight;
        u32     mWindowWidth;
        u32     mWindowHeight;

    };

    using WindowsWindowModuleImpl = PImplCombine<WindowModule, WindowsWindowModulePrivate>;

    //globals
    WindowsWindowModuleImpl* gWindowsWindowModule;

    //implementations
    void WindowModule::Initialize(const EngineInitArgMap& args)
    {
        MemoryModule::Get().Retain();
        WindowsWindowModuleImpl* self = WindowsWindowModuleImpl::GetCombinePtr(this);
        CheckAlways(gWindowsWindowModule == nullptr);
        gWindowsWindowModule = self;

        //here
        WNDCLASS wc;
        wc.style = 0;

        Module::Initialize(args);
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
    WindowsWindowModulePrivate::WindowsWindowModulePrivate(const EngineInitArgMap& args)
        : mWindow(NULL)
        , mBackbufferWidth(DefaultBackbufferWdith)
        , mBackbufferHeight(DefaultBackbufferHeight)
    {
        auto itr = args.find("--window-size");
        if (itr != args.end())
        {
            std::string s = itr->second;
            auto ix = s.find("x");
            if (ix != std::string::npos)
            {
                s[ix] = 0;
                mBackbufferWidth = atoi(s.data());
                mBackbufferHeight = atoi(s.data() + ix + 1);
            }
        }
        RECT rect = CalcNonClientWindowRect(0, 0, mBackbufferWidth, mBackbufferHeight);
        mWindowWidth = rect.right - rect.left;
        mWindowHeight = rect.bottom - rect.top;
    }
    RECT WindowsWindowModulePrivate::CalcNonClientWindowRect(u32 topLeftX, u32 topLeftY, u32 width, u32 height)
    {
        //windows window coordinate: https://docs.microsoft.com/en-us/windows/win32/gdi/window-coordinate-system
        //The x-coordinates increase to the right; y-coordinates increase from top to bottom.
        RECT rect;
        rect.left = topLeftX;
        rect.right = topLeftX + width;
        rect.top = topLeftY;
        rect.bottom = topLeftY + height;
        CheckAlways(AdjustWindowRectEx(&rect, OmniMainWindowStyle, false, OmniMainWIndowAdditionalStyle));
        return rect;
    }

    static Module* WindowModuleCtor(const EngineInitArgMap& args)
    {
        return new WindowsWindowModuleImpl(args);
    }
    ExportInternalModule(Window, ModuleExportInfo(WindowModuleCtor, true));
}
#endif
