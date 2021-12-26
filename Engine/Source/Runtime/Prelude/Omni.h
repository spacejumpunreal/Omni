#pragma once

#define NOMINMAX
#define _HAS_EXCEPTIONS 0
#include "Runtime/Prelude/MacroUtils.h"
#include <stddef.h>
#include <stdint.h>

////////////////////////////////platform
#define OMNI_WINDOWS _WIN64
#define OMNI_IOS __APPLE__
#define OMNI_ANDROID __ANDROID__
#define OMNI_ANY (OMNI_WINDOWS || OMNI_IOS || OMNI_ANDROID)

#if !OMNI_ANY
	#error("unknown platform")
#endif


////////////////////////////////compiler
#define OMNI_CLANG __clang__
#define OMNI_MSVC (!OMNI_CLANG) && _MSC_VER
#if !(OMNI_CLANG || OMNI_MSVC)
	#error("unknown compiler");
#endif

#define FORCEINLINE __forceinline


////////////////////////////////configs
#if _DEBUG
#define OMNI_DEBUG 1
#define _ITERATOR_DEBUG_LEVEL 0
#else
#define OMNI_DEBUG 0
#endif


////////////////////////////////aliases
namespace Omni
{
	using u64 = uint64_t;
	using i64 = int64_t;
	using u32 = uint32_t;
	using i32 = int32_t;
	using u16 = uint16_t;
	using i16 = int16_t;
	using u8 = uint8_t;
	using i8 = int8_t;
	using f32 = float;
	using f64 = double;
}

