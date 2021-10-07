#include "Runtime/Core/CorePCH.h"
#if OMNI_WINDOWS
#include "Runtime/Core/Platform/InputModule.h"
#include "Runtime/Base/Container/PMRContainers.h"
#include "Runtime/Base/Memory/MemoryArena.h"
#include "Runtime/Base/Misc/AssertUtils.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Base/Misc/PImplUtils.h"
#include "Runtime/Base/MultiThread/SpinLock.h"
#include "Runtime/Core/Allocator/MemoryModule.h"
#include "Runtime/Core/Platform/KeyMap.h"
#include "Runtime/Core/System/ModuleExport.h"
#include "Runtime/Core/System/ModuleImplHelpers.h"


#include <Windows.h>

namespace Omni
{
    //constants
    //declaration
    struct KeyState
    {
    public:
        KeyState()
            : Listeners(MemoryModule::Get().GetPMRAllocator(MemoryKind::SystemInit))
            , Pressed(false)
        {}
    public:
        PMRVector<KeyStateListener*>    Listeners;
        bool                            Pressed;
    };

    using KeyStateMap = PMRUnorderedMap<KeyCode, KeyState, std::hash<KeyCode>, std::equal_to<KeyCode>>;
    struct InputModulePrivate
    {
    public:
        InputModulePrivate();
    public:
        mutable SpinLock        mLock;
        KeyStateMap             mKeys;
        MousePos                mMousePos;
    };

    using InputModuleImpl = PImplCombine<InputModule, InputModulePrivate>;

    //globals
    InputModuleImpl* gInputModule;

    //impl
    InputModulePrivate::InputModulePrivate()
        : mKeys(MemoryModule::Get().GetPMRAllocator(MemoryKind::SystemInit))
        , mMousePos{}
    {}

    void InputModule::Initialize(const EngineInitArgMap& args)
    {
        gInputModule = (InputModuleImpl*)this;
        MemoryModule& mm = MemoryModule::Get();
        mm.Retain();

        Module::Initialize(args);
    }
    void InputModule::Finalize()
    {
        Module::Finalizing();
        if (GetUserCount() > 0)
            return;

#if OMNI_DEBUG
        const InputModuleImpl* self = InputModuleImpl::GetCombinePtr(this);
        for (auto& ksp: self->mKeys)
        {
            CheckAlways(ksp.second.Listeners.size() == 0);
        }
#endif
        MemoryModule& mm = MemoryModule::Get();
        gInputModule = nullptr;
        Module::Finalize();
        mm.Release();
    }
    InputModule& InputModule::Get()
    {
        return *gInputModule;
    }

    //user
    void InputModule::GetMousePos(MousePos& state) const
    {
        const InputModuleImpl* self = InputModuleImpl::GetCombinePtr(this);
        LockGuard lk(self->mLock);
        state = self->mMousePos;
    }
    void InputModule::GetKeyStates(u32 count, KeyCode* keys, bool* states) const
    {
        const InputModuleImpl* self = InputModuleImpl::GetCombinePtr(this);
        LockGuard lk(self->mLock);
        for (u32 i = 0; i < count; ++i)
        {
            auto it = self->mKeys.find(keys[i]);
            states[i] = (it == self->mKeys.end()) ? false : it->second.Pressed;
        }
    }
    void InputModule::RegisterListener(KeyCode key, KeyStateListener* listener) 
    {
        InputModuleImpl* self = InputModuleImpl::GetCombinePtr(this);
        LockGuard lk(self->mLock);
        self->mKeys[key].Listeners.push_back(listener);
    }
    void InputModule::UnRegisterlistener(KeyCode key, KeyStateListener* listener) 
    {
        InputModuleImpl* self = InputModuleImpl::GetCombinePtr(this);
        LockGuard lk(self->mLock);
        PMRVector<KeyStateListener*>& listeners = self->mKeys[key].Listeners;
        size_t i = 0;
        for (; i < listeners.size(); ++i)
            if (listeners[i] == listener)
                break;
        if (listeners.size() != i)
            listeners.erase(listeners.begin() + i);
    }
    //source
    void InputModule::UpdateMouse(MousePos& newPos, bool leftButton, bool rightButton)
    {
        InputModuleImpl* self = InputModuleImpl::GetCombinePtr(this);
        {
            LockGuard lk(self->mLock);
            self->mMousePos = newPos;
        }
        OnKeyEvent(KeyMap::MouseLeft, leftButton);
        OnKeyEvent(KeyMap::MouseRight, rightButton);
    }
    void InputModule::OnKeyEvent(KeyCode key, bool pressed) 
    {
        InputModuleImpl* self = InputModuleImpl::GetCombinePtr(this);
        ScratchStack& stk = MemoryModule::GetThreadScratchStack();
        MemoryArenaScope scope = stk.PushScope();
        KeyStateListener** listeners = nullptr;
        u32 toCall = 0;
        {
            LockGuard lk(self->mLock);
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
        return InitMemFactory<InputModuleImpl>::New();
    }
    void InputModule::Destroy()
    {
        InitMemFactory<InputModuleImpl>::Delete((InputModuleImpl*)this);
    }
    EXPORT_INTERNAL_MODULE(Input, ModuleExportInfo(InputModuleCtor, true, "Input"));

}
#endif//OMNI_WINDOWS
