#include "Runtime/Platform/InputModule.h"
#if OMNI_WINDOWS
#include "Runtime/Memory/MemoryModule.h"
#include "Runtime/Concurrency/SpinLock.h"
#include "Runtime/Memory/MemoryArena.h"
#include "Runtime/Misc/PImplUtils.h"
#include "Runtime/Misc/PMRContainers.h"
#include "Runtime/System/ModuleExport.h"
#include "Runtime/Test/AssertUtils.h"

#include <Windows.h>

namespace Omni
{
    //constants
    //declaration
    struct KeyState
    {
    public:
        KeyState()
            : Pressed(false)
        {}
    public:
        std::vector<KeyStateListener*>  Listeners;
        bool                            Pressed;
    };

    struct InputModulePrivate
    {
    public:
        InputModulePrivate()
            : mCursor{}
        {
        }
    public:
        SpinLock                                    Lock;
        PMRUnorderedMap<KeyCode, KeyState>          mKeys;
        CursorState                                 mCursor;
    };

    using InputModuleImpl = PImplCombine<InputModule, InputModulePrivate>;

    //globals
    InputModuleImpl* gInputModule;

    //impl
    void InputModule::Initialize(const EngineInitArgMap& args)
    {
        MemoryModule& mm = MemoryModule::Get();
        mm.Retain();

        Module::Initialize(args);
    }
    void InputModule::Finalize()
    {
        Module::Finalizing();
        if (GetUserCount() > 0)
            return;

        MemoryModule& mm = MemoryModule::Get();
        gInputModule = nullptr;
        Module::Finalize();
        mm.Release();
    }
    void InputModule::Finalizing()
    {
        Finalize();
    }
    InputModule& InputModule::Get()
    {
        return *gInputModule;
    }

    //user
    void InputModule::GetCursorState(CursorState& state)
    {
        InputModuleImpl* self = InputModuleImpl::GetCombinePtr(this);
        LockGuard lk(self->Lock);
        state = self->mCursor;
    }
    void InputModule::GetKeyStates(u32 count, KeyCode* keys, bool* states) 
    {
        InputModuleImpl* self = InputModuleImpl::GetCombinePtr(this);
        LockGuard lk(self->Lock);
        for (u32 i = 0; i < count; ++i)
        {
            auto it = self->mKeys.find(keys[i]);
            states[i] = (it == self->mKeys.end()) ? false : it->second.Pressed;
        }
    }
    void InputModule::RegisterListener(KeyCode key, KeyStateListener* listener) 
    {
        InputModuleImpl* self = InputModuleImpl::GetCombinePtr(this);
        LockGuard lk(self->Lock);
        self->mKeys[key].Listeners.push_back(listener);
    }
    void InputModule::UnRegisterlistener(KeyCode key, KeyStateListener* listener) 
    {
        InputModuleImpl* self = InputModuleImpl::GetCombinePtr(this);
        LockGuard lk(self->Lock);
        std::vector<KeyStateListener*> listeners = self->mKeys[key].Listeners;
        size_t i = 0;
        for (; i < listeners.size(); ++i)
            if (listeners[i] == listener)
                break;
        if (listeners.size() != i)
            listeners.erase(listeners.begin() + i);
    }
    //source
    void InputModule::UpdateCursorState(CursorState& newState) 
    {
        InputModuleImpl* self = InputModuleImpl::GetCombinePtr(this);
        LockGuard lk(self->Lock);
        self->mCursor = newState;
    }
    void InputModule::OnKeyEvent(KeyCode key, bool pressed) 
    {
        InputModuleImpl* self = InputModuleImpl::GetCombinePtr(this);
        ScratchStack& stk = MemoryModule::GetThreadScratchStack();
        MemoryArenaScope scope = stk.PushScope();
        KeyStateListener** listeners = nullptr;
        u32 toCall = 0;
        {
            LockGuard lk(self->Lock);
            KeyState& ks = self->mKeys[key];
            if (ks.Pressed != pressed)
            {
                toCall = (u32)ks.Listeners.size();
                u32 size = sizeof(KeyStateListener*) * toCall;
                listeners = (KeyStateListener**)stk.Allocate(size);
                memcpy(listeners, ks.Listeners.data(), size);
                ks.Pressed = pressed;
            }
        }
        if (toCall > 0)
        {
            for (u32 iCall = 0; iCall < toCall; ++iCall)
                listeners[iCall]->OnKeyEvent(key, pressed);
        }
    }

    //replay related
    void InputModule::StartRecording(void* ctx) 
    {
        (void)ctx;
    }
    void InputModule::StopRecording() 
    {
    }

    void InputModule::StartPlayback() {}
    void InputModule::StopPlayback() {}

    static Module* InputModuleCtor(const EngineInitArgMap&)
    {
        return new InputModule();
    }
    ExportInternalModule(Input, ModuleExportInfo(InputModuleCtor, true, "Input"));

}
#endif//OMNI_WINDOWS
