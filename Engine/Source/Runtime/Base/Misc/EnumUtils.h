#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include <type_traits>


#define DEFINE_ENUM_CLASS_OPS(EnumClass)\
    FORCEINLINE constexpr EnumClass operator|(EnumClass a, EnumClass b) { return (EnumClass)(std::underlying_type_t<EnumClass>(a) | std::underlying_type_t<EnumClass>(b)); }\
    FORCEINLINE constexpr EnumClass operator&(EnumClass a, EnumClass b) { return (EnumClass)(std::underlying_type_t<EnumClass>(a) & std::underlying_type_t<EnumClass>(b)); }\
    FORCEINLINE bool Any(EnumClass a) { return std::underlying_type_t<EnumClass>(a) != 0; }



