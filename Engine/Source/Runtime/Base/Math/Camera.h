#pragma once
#include "Runtime/Prelude/Omni.h"
#include "Runtime/Base/BaseAPI.h"
#include "Runtime/Base/Math/Vector3.h"
#include "Runtime/Base/Math/Matrix4x4.h"

namespace Omni
{
class Camera
{
public:
    BASE_API Camera();
    BASE_API Camera(float aspect, float vFOV, const Vector3& initPos, float initPitch, float initYaw, float near, float far);
    BASE_API void GetViewProjectionMatrix(Matrix4x4& outVPMatrix);
    BASE_API void LocalMove(const Vector3& xyz);
    BASE_API void LocalRotate(float pitch, float yaw);
    BASE_API void SetPosition(const Vector3& xyz);
    BASE_API void SetRotation(float pitch, float yaw);
    BASE_API void GetAxis(Vector3 axis[3]);
    BASE_API void GetProjectionMatrix(Matrix4x4& projMatrix);

private:
    Vector3 mPosition;
    float   mPitch;
    float   mYaw;
    float   mFOV;
    float   mAspect;
    float   mNear;
    float   mFar;
};
} // namespace Omni
