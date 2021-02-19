#pragma once

#define NOMINMAX
#include <stddef.h>
#include <stdint.h>

namespace Omni
{
	//typedef
	using u64 = uint64_t;
	using i64 = int64_t;
	using u32 = uint32_t;
	using i32 = int32_t;
	using u16 = uint16_t;
	using i16 = int16_t;
	using u8 = uint8_t;
	using i8 = int8_t;
	using usize = size_t;
}

#define OMNI_WINDOWS _WIN64
#define OMNI_IOS __APPLE__
#define OMNI_ANDROID __ANDROID__

#ifdef _MSC_VER
#define OMNI_MSVC 1
#elif __clang__
#define OMNI_CLANG 1
#endif

#ifdef OMNI_MSVC
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE inline __attribute__((always_inline))
#endif

#if OMNI_IOS
    #define OMNI_DEFAULT_ALIGNMENT 16
#elif OMNI_ANDROID
    #define OMNI_DEFAULT_ALIGNMENT 16
#else//OMNI_WINDOWS and others
	#define OMNI_DEFAULT_ALIGNMENT 8
#endif

#ifdef NDEBUG
#define OMNI_DEBUG 0
#else
#define OMNI_DEBUG 1
#endif

//prm related stuff
#if OMNI_CLANG
#define STD_PMR_NS std::experimental::pmr
#else
#define STD_PMR_NS std::pmr
#endif

#define PMR_IS_EXPERIMENTAL OMNI_CLANG
