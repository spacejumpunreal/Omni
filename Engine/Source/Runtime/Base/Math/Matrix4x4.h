#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Prelude/SuppressWarning.h"
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Base/Math/Vector4.h"
#include "Runtime/Base/Math/SepcialFunctions.h"


namespace Omni
{

struct alignas(64) Matrix4x4
{ // row major matrix
public:
    OMNI_PUSH_WARNING()
    OMNI_SUPPRESS_WARNING_NAMELESS_UNION_STRUCT()
    union
    {
        struct
        {
            Vector4 Row0;
            Vector4 Row1;
            Vector4 Row2;
            Vector4 Row3;
        };
        Vector4 Rows[4];
        struct
        {
            float M00;
            float M01;
            float M02;
            float M03;
            float M10;
            float M11;
            float M12;
            float M13;
            float M20;
            float M21;
            float M22;
            float M23;
            float M30;
            float M31;
            float M32;
            float M33;
        };
        float Elements[16];
    };
    OMNI_POP_WARNING()

public:
    FORCEINLINE static Matrix4x4 Identity(float a = 1)
    {
        Matrix4x4 r;
        r.M00 = a;
        r.M01 = 0.0f;
        r.M02 = 0.0f;
        r.M03 = 0.0f;
        r.M10 = 0.0f;
        r.M11 = a;
        r.M12 = 0.0f;
        r.M13 = 0.0f;
        r.M20 = 0.0f;
        r.M21 = 0.0f;
        r.M22 = a;
        r.M23 = 0.0f;
        r.M30 = 0.0f;
        r.M31 = 0.0f;
        r.M32 = 0.0f;
        r.M33 = a;
        return r;
    }
    FORCEINLINE static Matrix4x4 Empty()
    {
        return Identity(0);
    }
    FORCEINLINE static Matrix4x4 RotateY(float yaw)
    {
        auto r = Identity();
        auto cos = Mathf::Cosf(yaw);
        auto sin = Mathf::Sinf(yaw);
        r.M00 = cos;
        r.M02 = -sin;
        r.M20 = sin;
        r.M22 = cos;
        return r;
    }
    FORCEINLINE static Matrix4x4 RotateX(float pitch)
    {
        auto r = Identity();
        float cos = Mathf::Cosf(pitch);
        float sin = Mathf::Sinf(pitch);
        r.M11 = cos;
        r.M12 = -sin;
        r.M21 = sin;
        r.M22 = cos;
        return r;
    }
    FORCEINLINE Vector3& Translation()
    {
        return *(Vector3*)&Row3;
    }
    FORCEINLINE const Vector3& Translation() const
    {
        return *(Vector3*)&Row3;
    }
    FORCEINLINE Vector3& Axis(u32 axis)
    {
        return *(Vector3*)&Rows[axis];
    }
    FORCEINLINE const Vector3& Axis(u32 axis) const
    {
        return *(Vector3*)&Rows[axis];
    }
    FORCEINLINE Vector4& Row(u32 r)
    {
        return Rows[r];
    }
    FORCEINLINE const Vector4& Row(u32 r) const 
    {
        return Rows[r];
    }
    FORCEINLINE Vector4 Column(u32 c) const
    {
        Vector4 ret;
        ret.X = Row0[c];
        ret.Y = Row1[c];
        ret.Z = Row2[c];
        ret.W = Row3[c];
        return ret;
    }
    FORCEINLINE static Matrix4x4 Translate(const Vector3& position)
    {
        auto r = Identity();
        r.Translation() = position;
        return r;
    }
    FORCEINLINE Matrix4x4 Transpose()
    {
        Matrix4x4 r;
        for (u32 i = 0; i < 4; ++i)
            r.Rows[i] = Column(i);
        return r;
    }
    FORCEINLINE friend Matrix4x4 operator*(const Matrix4x4& l, const Matrix4x4& r)
    {
        Matrix4x4 ret;
        for (int y = 0; y < 4; y++)
        {
            const Vector4& a = l.Rows[y];
            for (int x = 0; x < 4; x++)
            {
                Vector4 b = r.Column(x);
                ret.Rows[y][x] = Dot(a, b);
            }
        }
        return ret;
    }
    FORCEINLINE friend Matrix4x4 operator+(const Matrix4x4& l, const Matrix4x4& r)
    {
        Matrix4x4 ret;
        for (u32 i = 0; i < 16; i++)
            ret.Elements[i] = l.Elements[i] + r.Elements[i];
        return ret;
    }
    FORCEINLINE friend Matrix4x4 operator-(const Matrix4x4& l, const Matrix4x4& r)
    {
        Matrix4x4 ret;
        for (u32 i = 0; i < 16; i++)
            ret.Elements[i] = l.Elements[i] - r.Elements[i];
        return ret;
    }
    FORCEINLINE friend Vector4 operator*(const Vector4& l, const Matrix4x4& r)
    {
        Vector4 ret;
        for (u32 i = 0; i < 4; ++i)
        {
            ret[i] = Dot(l[i], r.Column(i));
        }
        return ret;
    }
};
} // namespace Omni
