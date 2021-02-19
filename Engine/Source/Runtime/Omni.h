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


#ifdef NDEBUG
#define OMNI_DEBUG 0
#else
#define OMNI_DEBUG 1
#endif

#define OMNI_WINDOWS _WIN64
#define OMNI_IOS __APPLE__
#define OMNI_ANDROID __ANDROID__

#if __MSVC__
#define FORCEINLINE __forceinline
#else
#define FORCEINLINE __attribute__((always_inline))
#endif

#if OMNI_IOS
    #define OMNI_DEFAULT_ALIGNMENT 16
#elif OMNI_ANDROID
    #define OMNI_DEFAULT_ALIGNMENT 16
#else//OMNI_WINDOWS and others
	#define OMNI_DEFAULT_ALIGNMENT 8
#endif

#define EXPERIMENTAL_MEMORY_RESOURCE __clang__
