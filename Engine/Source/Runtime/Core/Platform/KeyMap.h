#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Core/CoreAPI.h"
#include "Runtime/Core/Platform/InputDefs.h"

namespace Omni
{
    namespace KeyMapInternal
    {
        static constexpr KeyCode OmniExtension = 0xffff0000;
    }
    namespace KeyMap
    {
        using KeyCode = u32;
        static constexpr KeyCode MouseLeft = KeyMapInternal::OmniExtension + 0x0001;
        static constexpr KeyCode MouseRight = KeyMapInternal::OmniExtension + 0x0002;

        static constexpr KeyCode Key_A = 'A';
        static constexpr KeyCode Key_0 = '0';
    }
}
