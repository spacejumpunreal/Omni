#pragma once
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Prelude/Omni.h"
#include <cmath>
#if OMNI_WINDOWS
#include <intrin.h>
#endif

namespace Omni
{
struct Mathf
{
    static constexpr float PI = 3.141592f;

    template<typename T>
    FORCEINLINE static T Max(T a, T b)
    {
        return a > b ? a : b;
    }
    template <typename T>
    FORCEINLINE static T Min(T a, T b)
    {
        return a > b ? b : a;
    }
    FORCEINLINE static float Sqrtf(float f)
    {
        return std::sqrtf(f);
    }
    FORCEINLINE static bool IsNan(float f)
    {
        return std::isnan(f);
    }
    template<typename T>
    FORCEINLINE static T Abs(T f)
    {
        return std::abs(f);
    }
    FORCEINLINE static u32 Lzcnt64(u64 bits)
    {
#if OMNI_MSVC
        return (u32)__lzcnt64(bits);
#else
        return (u32)__builtin_clzll(bits);
#endif
    }
    FORCEINLINE static i32 FindMostSignificant1Bit(u64 bits)
    {
        return 63 - Lzcnt64(bits);
    }
    FORCEINLINE static bool IsSingleBitSet(u64 bits)
    {
        if (bits == 0)
            return false;
        i32 pos = FindMostSignificant1Bit(bits);
        return bits == (1uLL << pos);
    }
    FORCEINLINE static float Sinf(float a)
    {
        return std::sinf(a);
    }
    FORCEINLINE static float Cosf(float a)
    {
        return std::cosf(a);
    }
    FORCEINLINE static float Tanf(float a)
    {
        return std::tanf(a);
    }
    FORCEINLINE static float Cotf(float a)
    {
        return 1.0f / std::tanf(a);
    }
    FORCEINLINE static float ToRad(float degree)
    {
        return degree * (Mathf::PI / 180.0f);
    }
};
} // namespace Omni
