#pragma once
#include "Runtime/Omni.h"
#include "Runtime/Platform/InputDefs.h"

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
    }
}