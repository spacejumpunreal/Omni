#include "Runtime/Base/BasePCH.h"
#include "Runtime/Base/Math/Camera.h"

namespace Omni
{
Camera::Camera() : mPosition(0, 0, 0), mPitch(0), mYaw(0), mFOV(0), mAspect(0), mNear(0), mFar(0)
{
}
Camera::Camera(float aspect, float vFOV, const Vector3& initPos, float initPitch, float initYaw, float near, float far)
    : mPosition(initPos)
    , mPitch(initPitch)
    , mYaw(initYaw)
    , mFOV(vFOV)
    , mAspect(aspect)
    , mNear(near)
    , mFar(far)
{
}
void Camera::GetViewProjectionMatrix(Matrix4x4& outVPMatrix)
{
    Vector3 xyz[3];
    GetAxis(xyz);

    Matrix4x4 world2ViewT, view2WorldR, world2ViewR, world2View;
    view2WorldR.Row0 = Vector4(xyz[0], 0);
    view2WorldR.Row1 = Vector4(xyz[1], 0);
    view2WorldR.Row2 = Vector4(xyz[2], 0);
    view2WorldR.Row3 = Vector4(0, 0, 0, 1);
    world2ViewR = view2WorldR.Transpose();
    world2ViewT = Matrix4x4::Translate(-mPosition);
    world2View = world2ViewT * world2ViewR;//inverse of view2World

    Matrix4x4 view2Proj;
    GetProjectionMatrix(view2Proj);
    outVPMatrix = world2View * view2Proj;
}
void Camera::LocalMove(const Vector3& xyz)
{
    Vector3 axis[3];
    GetAxis(axis);
    mPosition += axis[0] * xyz.X + axis[1] * xyz.Y + axis[2] * xyz.Z;
}
void Camera::LocalRotate(float pitch, float yaw)
{
    mPitch += pitch;
    mYaw += yaw;
}
void Camera::SetPosition(const Vector3& xyz)
{
    mPosition = xyz;
}
void Camera::SetRotation(float pitch, float yaw)
{
    mPitch = pitch;
    mYaw = yaw;
}
void Camera::GetAxis(Vector3 axis[3])
{
    float cosYaw = Mathf::Cosf(mYaw);
    float sinYaw = Mathf::Sinf(mYaw);
    float cosPitch = Mathf::Cosf(mPitch);
    float sinPitch = Mathf::Sinf(mPitch);
    axis[0] = Vector3(cosYaw, 0, -sinYaw);
    axis[2] = Vector3(sinYaw * cosPitch, -sinPitch, cosYaw * cosPitch);
    axis[1] = Vector3(sinYaw * sinPitch, cosPitch, cosYaw * sinPitch);
}
void Camera::GetProjectionMatrix(Matrix4x4& projMatrix)
{//https://blog.csdn.net/killer4747/article/details/89072810
    projMatrix = Matrix4x4::Identity();
    float yScale = Mathf::Cotf(mFOV / 2);
    float xScale = yScale / mAspect;
    projMatrix.M00 = xScale;
    projMatrix.M11 = yScale;
    projMatrix.M22 = mFar / (mFar - mNear);
    projMatrix.M23 = 1.0f;
    projMatrix.M32 = -mFar * mNear / (mFar - mNear);
}

} // namespace Omni
