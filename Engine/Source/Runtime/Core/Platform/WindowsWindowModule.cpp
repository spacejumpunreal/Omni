#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Base/MultiThread/SpinLock.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Concurrency/ConcurrencyModule.h"
#include "Runtime/Core/Platform/InputModule.h"
#include "Runtime/Core/Platform/WindowModule.h"
#include "Runtime/Core/System/ModuleExport.h"
#include "Runtime/Core/System/ModuleImplHelpers.h"

#include <Windows.h>
#include <functional>
#include <future>

namespace Omni
{
    //constants
    static constexpr DWORD OmniMainWindowStyle = WS_OVERLAPPEDWINDOW;
    static constexpr DWORD OmniMainWindowAdditionalStyle = 0;
    static constexpr u32 DefaultBackbufferWdith = 800;
    static constexpr u32 DefaultBackbufferHeight = 600;
    static constexpr u32 OmniWindowResizeMinPixels = 32;
    static const wchar_t* OmniWindowClassName = L"OmniWindowClass";

    void ResumeUIThread(std::function<void()>&& body);

    //definitions
    struct InitUIThreadArgs
    {
        const EngineInitArgMap*     Args;
        ThreadData*                 ThreadData;
        std::promise<void>*         ReadyFlag;
    };

    struct WindowsWindowModulePrivate
    {
    public:
        WindowsWindowModulePrivate(const EngineInitArgMap& args);
        ~WindowsWindowModulePrivate();
        static RECT CalcNonClientWindowRect(u32 topLeftX, u32 topLeftY, u32 width, u32 height);
        void OnSizeChanged(u32 clientWidth, u32 clientHeight);
        void RunUILoop(InitUIThreadArgs& args);
    public:
        struct MainThreadData
        {
            SpinLock    mMainThreadDataLock;
            u32         mBackbufferWidth;
            u32         mBackbufferHeight;
            u32         mWindowWidth;
            u32         mWindowHeight;
        };
        MainThreadData*         mMainThreadData;
        std::promise<void>      mCleanupFlag;
        HWND                    mWindow;
    };

    using WindowsWindowModuleImpl = PImplCombine<WindowModule, WindowsWindowModulePrivate>;

    struct UpdateMouseOnMain
    {
        MousePos MousePos;
        bool LDown, RDown;
        static void Run(UpdateMouseOnMain* arg)
        {
            InputModule& inputModule = InputModule::Get();
            inputModule.UpdateMouse(arg->MousePos, arg->LDown, arg->RDown);
        }
    };

    //implementations
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
        {//currently limiting windows width x height to OmniWindowResizeMinPixels alignment
            RECT* rect = (RECT*)lParam;
            
            u32 width = rect->right - rect->left;
            u32 height = rect->bottom - rect->top;
            width &= ~(OmniWindowResizeMinPixels - 1);
            height &= ~(OmniWindowResizeMinPixels - 1);

            bool moveBottom = true, moveRight = true;
            
            switch (wParam)
            {
            case WMSZ_BOTTOM:
                moveBottom = true;
                break;
            case WMSZ_BOTTOMLEFT:
                moveBottom = true;
                moveRight = false;
                break;
            case WMSZ_BOTTOMRIGHT:
                moveBottom = true;
                moveRight = true;
                break;
            case WMSZ_LEFT:
                moveRight = false;
                break;
            case WMSZ_RIGHT:
                moveRight = true;
                break;
            case WMSZ_TOP:
                moveBottom = false;
                break;
            case WMSZ_TOPLEFT:
                moveBottom = false;
                moveRight = false;
                break;
            case WMSZ_TOPRIGHT:
                moveBottom = false;
                moveRight = true;
                break;
            default:
                break;
            }

            if (moveRight)
                rect->right = rect->left + width;
            else
                rect->left = rect->right - width;

            if (moveBottom)
                rect->bottom = rect->top + height;
            else
                rect->top = rect->bottom - height;
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
        {//notify message loop to quit
            WindowsWindowModuleImpl* self = (WindowsWindowModuleImpl*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            self->mWindow = NULL;
            PostQuitMessage(0);
            break;
        }
        case WM_MOUSEMOVE:
        case WM_LBUTTONUP:
        case WM_LBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDOWN:
        {
            POINT lp;
            UpdateMouseOnMain updateMouseOnMain;

            CheckDebug(GetCursorPos(&lp));
            SHORT bs;
            bs = GetKeyState(VK_LBUTTON);
            updateMouseOnMain.LDown = (bs & 0x8000) != 0;
            bs = GetKeyState(VK_RBUTTON);
            updateMouseOnMain.RDown = (bs & 0x8000) != 0;

            WindowsWindowModuleImpl* self = (WindowsWindowModuleImpl*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
            CheckDebug(ScreenToClient(self->mWindow, &lp));
            updateMouseOnMain.MousePos = MousePos { .X = (i16)lp.x, .Y = (i16)lp.y };

            DispatchWorkItem& dispatchWork = DispatchWorkItem::CreateWithFunctor(std::move(updateMouseOnMain), MemoryKind::CacheLine, true);
            ConcurrencyModule::Get().EnqueueWork(dispatchWork, QueueKind::Main);
            break;
        }
        default:
            break;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    void WindowModule::Initialize(const EngineInitArgMap& args)
    {
        MemoryModule::Get().Retain();
        WindowsWindowModuleImpl* self = WindowsWindowModuleImpl::GetCombinePtr(this);
        CheckAlways(gWindowModule == nullptr);
        gWindowModule = self;
        
        std::promise<void> readyFlag;
        InitUIThreadArgs initArgs;
        initArgs.Args = &args;
        initArgs.ThreadData = ConcurrencyModule::Get().RegisterExternalThread(UIThreadId);
        initArgs.ReadyFlag = &readyFlag;

        ResumeUIThread(std::function<void()>([=]() {
            ThreadData* sd = initArgs.ThreadData;
            sd->InitializeOnThread();
            auto x = initArgs;
            self->RunUILoop(x);
            sd->FinalizeOnThread();
            self->mCleanupFlag.set_value();
            }));
        initArgs.ReadyFlag->get_future().wait();
        Module::Initialize(args);
    }
    void WindowModule::StopThreads()
    {
        WindowsWindowModuleImpl* self = WindowsWindowModuleImpl::GetCombinePtr(this);
        PostMessage(self->mWindow, WM_CLOSE, 0, 0);
        self->mCleanupFlag.get_future().wait();

    }
    void WindowModule::Finalize()
    {
        Module::Finalizing();
        if (GetUserCount() > 0)
            return;

        gWindowModule = nullptr;
        WindowsWindowModuleImpl* self = WindowsWindowModuleImpl::GetCombinePtr(this);
        CheckAlways(self->mWindow == NULL);
        MemoryModule::Get().Release();
        Module::Finalize();
    }
    void WindowModule::GetBackbufferSize(u32& width, u32& height)
    {
        WindowsWindowModuleImpl* self = WindowsWindowModuleImpl::GetCombinePtr(this);
        self->mMainThreadData->mMainThreadDataLock.Lock();
        width = self->mMainThreadData->mBackbufferWidth;
        height = self->mMainThreadData->mBackbufferHeight;
        self->mMainThreadData->mMainThreadDataLock.Unlock();
    }
    void WindowModule::RequestSetBackbufferSize(u32 width, u32 height)
    {//can be called on any thread, for it will send a request to Window's internal message queue
        WindowsWindowModuleImpl* self = WindowsWindowModuleImpl::GetCombinePtr(this);
        RECT rect{};
        CheckWinAPI(GetWindowRect(self->mWindow, &rect));
        rect.right = rect.left + width;
        rect.bottom = rect.top + height;
        CheckWinAPI(AdjustWindowRectEx(&rect, OmniMainWindowStyle, false, OmniMainWindowAdditionalStyle));
        CheckWinAPI(SetWindowPos(self->mWindow, HWND_TOPMOST, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOCOPYBITS));
    }
    WindowHandle WindowModule::GetMainWindowHandle()
    {
        WindowsWindowModuleImpl* self = WindowsWindowModuleImpl::GetCombinePtr(this);
        return self->mWindow;
    }
    WindowsWindowModulePrivate::WindowsWindowModulePrivate(const EngineInitArgMap&)
        : mMainThreadData(OMNI_NEW(MemoryKind::SystemInit)(MainThreadData))
        , mWindow(NULL)
    {
        mMainThreadData->mBackbufferWidth = 0;
        mMainThreadData->mBackbufferHeight = 0;
        mMainThreadData->mWindowWidth = 0;
        mMainThreadData->mWindowHeight = 0;
    }
    WindowsWindowModulePrivate::~WindowsWindowModulePrivate()
    {
        OMNI_DELETE(mMainThreadData, MemoryKind::SystemInit);
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
        RECT rect = CalcNonClientWindowRect(0, 0, clientWidth, clientHeight);
        u32 windowWidth = rect.right - rect.left; ;
        u32 windowHeight = rect.bottom - rect.top;
        self->mMainThreadData->mMainThreadDataLock.Lock();
        self->mMainThreadData->mBackbufferWidth = clientWidth;
        self->mMainThreadData->mBackbufferHeight = clientHeight;
        self->mMainThreadData->mWindowWidth = windowWidth;
        self->mMainThreadData->mWindowHeight = windowHeight;
        self->mMainThreadData->mMainThreadDataLock.Unlock();
    }
    void WindowsWindowModulePrivate::RunUILoop(InitUIThreadArgs& initArgs)
    {
        WindowsWindowModuleImpl* self = WindowsWindowModuleImpl::GetCombinePtr(this);

        HINSTANCE instance = GetModuleHandle(NULL);
        //here
        WNDCLASS wc = {};
        wc.lpfnWndProc = OmniWindowProc;
        wc.lpszClassName = OmniWindowClassName;
        wc.hInstance = instance;
        RegisterClass(&wc);

        auto itr = initArgs.Args->find("--window-size");
        u32 bfw = DefaultBackbufferWdith;
        u32 bfh = DefaultBackbufferWdith;
        if (itr != initArgs.Args->end())
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

        initArgs.ReadyFlag->set_value();

        MSG msg;
        BOOL bRet;
        while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
        {
            CheckDebug(bRet != -1);
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        System::GetSystem().TriggerFinalization(false);
    }

    static Module* WindowModuleCtor(const EngineInitArgMap& args)
    {
        return InitMemFactory<WindowsWindowModuleImpl>::New(args);
    }
    void WindowModule::Destroy()
    {
        InitMemFactory<WindowsWindowModuleImpl>::Delete((WindowsWindowModuleImpl*)this);
    }
    EXPORT_INTERNAL_MODULE(Window, ModuleExportInfo(WindowModuleCtor, false, "Window"));
}
#endif//OMNI_WINDOWS
