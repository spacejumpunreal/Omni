#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include <type_traits>


#define DEFINE_BITMASK_ENUM_OPS(EnumClass)\
    FORCEINLINE constexpr EnumClass operator|(EnumClass a, EnumClass b) { return (EnumClass)(std::underlying_type_t<EnumClass>(a) | std::underlying_type_t<EnumClass>(b)); }\
    FORCEINLINE constexpr EnumClass operator&(EnumClass a, EnumClass b) { return (EnumClass)(std::underlying_type_t<EnumClass>(a) & std::underlying_type_t<EnumClass>(b)); }\
    FORCEINLINE constexpr bool Any(EnumClass a) { return std::underlying_type_t<EnumClass>(a) != 0; }


namespace Omni
{
    template<typename MaskT, typename ...Args>
    MaskT BuildMask(Args ...args)
    {
        MaskT one = (MaskT)1;
        return ((one << args) | ...);
    }
}


