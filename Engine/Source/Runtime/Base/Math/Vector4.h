#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Base/Math/Vector3.h"
#include "Runtime/Base/Math/SepcialFunctions.h"

namespace Omni
{
struct Vector4
{
public:
    float X, Y, Z, W;

public:
    Vector4() = default;
    Vector4(const Vector3& v3, float w = 1.0f) : X(v3.X), Y(v3.Y), Z(v3.Z), W(w)
    {
    }
    Vector4(float x, float y = 0.0f, float z = 0.0f, float w = 1.0f) : X(x), Y(y), Z(z), W(w)
    {
    }
    float& operator[](i32 compIdx)
    {
        return (&X)[compIdx];
    }
    float operator[](i32 compIdx) const
    {
        return (&X)[compIdx];
    }
    Vector4 operator-() const
    {
        return Vector4(-X, -Y, -Z, -W);
    }
    Vector4 operator+(const Vector4& v) const
    {
        return Vector4(X + v.X, Y + v.Y, Z + v.Z, W + v.W);
    }
    Vector4 operator+=(const Vector4& v)
    {
        X += v.X;
        Y += v.Y;
        Z += v.Z;
        W += v.W;
        return *this;
    }
    Vector4 operator-(const Vector4& v) const
    {
        return Vector4(X - v.X, Y - v.Y, Z - v.Z, W - v.W);
    }
    Vector4 operator-=(const Vector4& v)
    {
        X -= v.X;
        Y -= v.Y;
        Z -= v.Z;
        W -= v.W;
        return *this;
    }
    Vector4 operator*(float scale) const
    {
        return Vector4(X * scale, Y * scale, Z * scale, W * scale);
    }
    Vector4 operator*(const Vector4& scaleV) const
    {
        return Vector4(X * scaleV.X, Y * scaleV.Y, Z * scaleV.Z, W * scaleV.W);
    }
    Vector4 operator*=(const Vector4& scaleV)
    {
        X *= scaleV.X;
        Y *= scaleV.Y;
        Z *= scaleV.Z;
        W *= scaleV.W;
    }
    Vector4 operator/(float scale) const
    {
        float rScale = 1.0f / scale;
        return Vector4(X * rScale, Y * rScale, Z * rScale, W * rScale);
    }
    Vector4 operator/(const Vector4& scaleV) const
    {
        return Vector4(X / scaleV.X, Y / scaleV.Y, Z / scaleV.Z, W / scaleV.W);
    }
    Vector4 operator/=(const Vector4& scaleV)
    {
        X *= scaleV.X;
        Y *= scaleV.Y;
        Z *= scaleV.Z;
        W *= scaleV.W;
        return *this;
    }
    friend float Dot(const Vector4& lhs, const Vector4 rhs)
    {
        return lhs.X * rhs.X + lhs.Y * rhs.Y + lhs.Z * rhs.Z + lhs.W * rhs.W;
    }
    friend Vector4 Cross(const Vector4 lhs, const Vector4 rhs)
    {
        return Vector4(lhs.Y * rhs.Z - lhs.Z * rhs.Y,
            lhs.Z * rhs.X - lhs.X * rhs.Z,
            lhs.X * rhs.Y - lhs.Y * rhs.X,
            0.0f);
    }
    bool operator==(const Vector4 v) const
    {
        return X == v.X && Y == v.Y && Z == v.Z && W == v.W;
    }
    bool operator!=(const Vector4 v) const
    {
        return X != v.X || Y != v.Y || Z != v.Z || W != v.W;
    }
    float Length() const
    {
        return Mathf::Sqrtf(X * X + Y * Y + Z * Z + W * W);
    }
    float LengthSquared() const
    {
        return X * X + Y * Y + Z * Z + W * W;
    }
    void Normalize()
    {
        float rLength = 1.0f / Length();
        this->operator*(rLength);
    }
    Vector4 Normalized() const
    {
        float rLength = 1.0f / Length();
        return (*this) * rLength;
    }
    bool ContainsNan() const
    {
        return Mathf::IsNan(X) || Mathf::IsNan(Y) || Mathf::IsNan(Z) || Mathf::IsNan(W);
    }
    bool Equals(Vector4 v, float tolerance) const
    {
        return Mathf::Abs(v.X - X) <= tolerance && Mathf::Abs(v.Y - Y) <= tolerance &&
               Mathf::Abs(v.Z - Z) <= tolerance && Mathf::Abs(v.W - W) <= tolerance;
    }
};
} // namespace Omni
