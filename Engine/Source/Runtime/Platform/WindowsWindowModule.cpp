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
    static constexpr DWORD OmniMainWindowAdditionalStyle = 0;
    static constexpr u32 DefaultBackbufferWdith = 800;
    static constexpr u32 DefaultBackbufferHeight = 600;
    static constexpr u32 OmniWindowResizeMinPixels = 32;
    static const wchar_t* OmniWindowClassName = L"OmniWindowClass";

    static int destroyTimes = 0;
    //declarations
    struct WindowsWindowModulePrivate
    {
    public:
        WindowsWindowModulePrivate(const EngineInitArgMap& args);
        static RECT CalcNonClientWindowRect(u32 topLeftX, u32 topLeftY, u32 width, u32 height);
        void OnSizeChanged(u32 clientWidth, u32 clientHeight);
        void SyncWindowSize(bool backbufferSizeValid);
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

    static LRESULT CALLBACK OmniWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        switch (uMsg)
        {
        case WM_CREATE:
        {
            CREATESTRUCTA* cs = (CREATESTRUCTA*)lParam;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (i64)cs->lpCreateParams);
            break;
        }
        case WM_SIZING:
        {
            RECT* rect = (RECT*)lParam;
            u32 width = rect->right - rect->left;
            u32 height = rect->bottom - rect->top;
            width &= ~(OmniWindowResizeMinPixels - 1);
            height &= ~(OmniWindowResizeMinPixels - 1);
            rect->right = rect->left + width;
            rect->bottom = rect->top + height;
            break;
        }
        case WM_SIZE:
        {
            u32 clientWidth = (u32)LOWORD(lParam);
            u32 clientHeight = (u32)HIWORD(lParam);
            WindowsWindowModuleImpl* self = (WindowsWindowModuleImpl*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            self->OnSizeChanged(clientWidth, clientHeight);
            break;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            break;
        }
        default:
            break;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }


    //implementations
    void WindowModule::Initialize(const EngineInitArgMap& args)
    {
        MemoryModule::Get().Retain();
        WindowsWindowModuleImpl* self = WindowsWindowModuleImpl::GetCombinePtr(this);
        CheckAlways(gWindowsWindowModule == nullptr);
        gWindowsWindowModule = self;

        HINSTANCE instance = GetModuleHandle(NULL);
        //here
        WNDCLASS wc = {};
        wc.lpfnWndProc = OmniWindowProc;
        wc.lpszClassName = OmniWindowClassName;
        wc.hInstance = instance;
        RegisterClass(&wc);

        auto itr = args.find("--window-size");
        u32 bfw = DefaultBackbufferWdith;
        u32 bfh = DefaultBackbufferWdith;
        if (itr != args.end())
        {
            std::string s = itr->second;
            auto ix = s.find("x");
            if (ix != std::string::npos)
            {
                s[ix] = 0;
                bfw = atoi(s.data());
                bfh = atoi(s.data() + ix + 1);
            }
        }
        self->mWindow = CreateWindowEx(
            OmniMainWindowAdditionalStyle,              // Optional window styles.
            OmniWindowClassName,                        // Window class
            L"Learn to Program Windows",                // Window text
            OmniMainWindowStyle,                        // Window style
            CW_USEDEFAULT, CW_USEDEFAULT, bfw, bfh,     // Size and position
            NULL,                                       // Parent window    
            NULL,                                       // Menu
            instance,                                   // Instance handle
            self                                        // Additional application data
        );
        CheckAlways(self->mWindow != NULL, "create window failed");
        ShowWindow(self->mWindow, SW_SHOW);

        Module::Initialize(args);
    }
    void WindowModule::Finalize()
    {
        Module::Finalizing();
        if (GetUserCount() > 0)
            return;

        WindowsWindowModuleImpl* self = WindowsWindowModuleImpl::GetCombinePtr(this);
        self->mWindow = NULL;
        MemoryModule::Get().Release();
        Module::Finalize();
    }
    void WindowModule::Finalizing()
    {
        Finalize();
    }
    WindowModule& WindowModule::Get()
    {
        return *gWindowsWindowModule;
    }
    WindowModule* WindowModule::GetPtr()
    {
        return gWindowsWindowModule;
    }

    void WindowModule::RunUILoop()
    {
        MSG msg;
        BOOL bRet;
        while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
        {
            CheckDebug(bRet != -1);
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        System::GetSystem().TriggerFinalization(true);
    }
    void WindowModule::GetBackbufferSize(u32& width, u32& height)
    {
        WindowsWindowModuleImpl* self = WindowsWindowModuleImpl::GetCombinePtr(this);
        width = self->mBackbufferWidth;
        height = self->mBackbufferHeight;
    }
    void WindowModule::RequestSetBackbufferSize(u32 width, u32 height)
    {
        WindowsWindowModuleImpl* self = WindowsWindowModuleImpl::GetCombinePtr(this);
        RECT rect{};
        CheckWinAPI(GetWindowRect(self->mWindow, &rect));
        rect.right = rect.left + width;
        rect.bottom = rect.top + height;
        CheckWinAPI(AdjustWindowRectEx(&rect, OmniMainWindowStyle, false, OmniMainWindowAdditionalStyle));
        CheckWinAPI(SetWindowPos(self->mWindow, HWND_TOPMOST, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOCOPYBITS));
    }
    WindowsWindowModulePrivate::WindowsWindowModulePrivate(const EngineInitArgMap&)
        : mWindow(NULL)
        , mBackbufferWidth(0)
        , mBackbufferHeight(0)
        , mWindowWidth(0)
        , mWindowHeight(0)
    {
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
        CheckWinAPI(AdjustWindowRectEx(&rect, OmniMainWindowStyle, false, OmniMainWindowAdditionalStyle));
        return rect;
    }
    void WindowsWindowModulePrivate::OnSizeChanged(u32 clientWidth, u32 clientHeight)
    {
        WindowsWindowModuleImpl* self = WindowsWindowModuleImpl::GetCombinePtr(this);
        self->mBackbufferWidth = clientWidth;
        self->mBackbufferHeight = clientHeight;
        SyncWindowSize(true);
    }
    void WindowsWindowModulePrivate::SyncWindowSize(bool backbufferSizeValid)
    {
        if (backbufferSizeValid)
        {
            RECT rect = CalcNonClientWindowRect(0, 0, mBackbufferWidth, mBackbufferHeight);
            mWindowWidth = rect.right - rect.left;
            mWindowHeight = rect.bottom - rect.top;
        }
        else
        {
            CheckAlways(false);
        }
    }
    static Module* WindowModuleCtor(const EngineInitArgMap& args)
    {
        return new WindowsWindowModuleImpl(args);
    }
    ExportInternalModule(Window, ModuleExportInfo(WindowModuleCtor, false, "Window"));
}
#endif
