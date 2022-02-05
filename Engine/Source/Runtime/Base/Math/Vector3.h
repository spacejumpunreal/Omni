#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Base/Math/SepcialFunctions.h"

namespace Omni
{

struct Vector3
{
public:
    float X, Y, Z;

public:
    Vector3(const Vector3& v3) : X(v3.X), Y(v3.Y), Z(v3.Z)
    {
    }
    Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f) : X(x), Y(y), Z(z)
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
    Vector3 operator-() const
    {
        return Vector3(-X, -Y, -Z);
    }
    Vector3 operator+(const Vector3& v) const
    {
        return Vector3(X + v.X, Y + v.Y, Z + v.Z);
    }
    Vector3 operator+=(const Vector3& v)
    {
        X += v.X;
        Y += v.Y;
        Z += v.Z;
        return *this;
    }
    Vector3 operator-(const Vector3& v) const
    {
        return Vector3(X - v.X, Y - v.Y, Z - v.Z);
    }
    Vector3 operator-=(const Vector3& v)
    {
        X -= v.X;
        Y -= v.Y;
        Z -= v.Z;
        return *this;
    }
    Vector3 operator*(float scale) const
    {
        return Vector3(X * scale, Y * scale, Z * scale);
    }
    Vector3 operator*(const Vector3& scaleV) const
    {
        return Vector3(X * scaleV.X, Y * scaleV.Y, Z * scaleV.Z);
    }
    Vector3 operator*=(const Vector3& scaleV)
    {
        X *= scaleV.X;
        Y *= scaleV.Y;
        Z *= scaleV.Z;
    }
    Vector3 operator/(float scale) const
    {
        float rScale = 1.0f / scale;
        return Vector3(X * rScale, Y * rScale, Z * rScale);
    }
    Vector3 operator/(const Vector3& scaleV) const
    {
        return Vector3(X / scaleV.X, Y / scaleV.Y, Z / scaleV.Z);
    }
    Vector3 operator/=(const Vector3& scaleV)
    {
        X *= scaleV.X;
        Y *= scaleV.Y;
        Z *= scaleV.Z;
        return *this;
    }
    friend float Dot(const Vector3& lhs, const Vector3 rhs)
    {
        return lhs.X * rhs.X + lhs.Y * rhs.Y + lhs.Z * rhs.Z;
    }
    friend Vector3 Cross(const Vector3 lhs, const Vector3 rhs)
    {
        return Vector3(lhs.Y * rhs.Z - lhs.Z * rhs.Y, lhs.Z * rhs.X - lhs.X * rhs.Z, lhs.X * rhs.Y - lhs.Y * rhs.X);
    }
    bool operator==(const Vector3 v) const
    {
        return X == v.X && Y == v.Y && Z == v.Z;
    }
    bool operator!=(const Vector3 v) const
    {
        return X != v.X || Y != v.Y || Z != v.Z;
    }
    float Length() const
    {
        return Mathf::Sqrtf(X * X + Y * Y + Z * Z);
    }
    float LengthSquared() const
    {
        return X * X + Y * Y + Z * Z;
    }
    void Normalize()
    {
        float rLength = 1.0f / Length();
        this->operator*(rLength);
    }
    Vector3 Normalized() const
    {
        float rLength = 1.0f / Length();
        return (*this) * rLength;
    }
    bool ContainsNan() const
    {
        return Mathf::IsNan(X) || Mathf::IsNan(Y) || Mathf::IsNan(Z);
    }
    bool Equals(Vector3 v, float tolerance) const
    {
        return Mathf::Abs(v.X - X) <= tolerance && Mathf::Abs(v.Y - Y) <= tolerance && Mathf::Abs(v.Z - Z) <= tolerance;
    }
};
} // namespace Omni
