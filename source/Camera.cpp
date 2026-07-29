#include "Camera.h"

#include <cmath>
#include <iostream>
#include <ostream>

namespace unboxing_engine {

static inline void mat4x4_ortho(float *out, float left, float right, float bottom, float top, float znear, float zfar) {
#define T(a, b) (a * 4 + b)

    out[T(0, 0)] = 2.0f / (right - left);
    out[T(0, 1)] = 0.0f;
    out[T(0, 2)] = 0.0f;
    out[T(0, 3)] = 0.0f;

    out[T(1, 1)] = 2.0f / (top - bottom);
    out[T(1, 0)] = 0.0f;
    out[T(1, 2)] = 0.0f;
    out[T(1, 3)] = 0.0f;

    out[T(2, 2)] = -2.0f / (zfar - znear);
    out[T(2, 0)] = 0.0f;
    out[T(2, 1)] = 0.0f;
    out[T(2, 3)] = 0.0f;

    out[T(3, 0)] = -(right + left) / (right - left);
    out[T(3, 1)] = -(top + bottom) / (top - bottom);
    out[T(3, 2)] = -(zfar + znear) / (zfar - znear);
    out[T(3, 3)] = 1.0f;

#undef T
}

std::string PrettyPrintVector3f(const Vector3f &v) {
    return std::string("(") + std::to_string(v.x) + ", " + std::to_string(v.y) + ", " + std::to_string(v.z) + ")";
}

void Camera::CastRayFromScreen(int scr_x, int scr_y, Vector3f &point, Vector3f &dir) const {
    //We calculate the ratio of mWindow coordinate by its respective axi size - i.g. scr_x/width
    //This ratio multiplies the near plane size in the same axi - i.g. 2*right.
    //The result is then subtracted by the plane origin to bring it to the world coordinate system - i.g. -left.

    // TopToBottom, LeftToRight pixel coordinate to parametric (0-1) coordinates of the screen
    float parametricX = (static_cast<float>(scr_x) / static_cast<float>(mWidth));
    float parametricY = (static_cast<float>(scr_y) / static_cast<float>(mHeight));

    float x = parametricX * (mRight - mLeft) - mRight;
    float y = -(parametricY * (mTop - mBottom) - mTop);
    float z = - mZNear;

    Vector3f screenProjection = Matrix4f::RotationMatrix(mRotation) *  Vector3f(x, y, z);

    point = screenProjection + mPosition;
    dir = screenProjection.Normalized();

    std::cout << "Camera Position = "<< PrettyPrintVector3f(mPosition) << std::endl;
    std::cout << "Start = "<< PrettyPrintVector3f(point) << std::endl;
    std::cout << "Direction = "<< PrettyPrintVector3f(dir) << std::endl;
}

Vector3f Camera::ScreenCoordinateToWorld(int scr_x, int scr_y, float zFar) const {
    Vector3f point, direction;
    CastRayFromScreen(scr_x, scr_y, point, direction);
    return point + direction * zFar;
}

void Camera::SetOrthographicIsometric(float viewWidth, float viewHeight, float depthRange, const Vector3f &focus, float yawDegrees, float pitchDegrees) {
    const float safeViewWidth = viewWidth == 0.0f ? 1.0f : viewWidth;
    const float safeViewHeight = viewHeight == 0.0f ? 1.0f : viewHeight;
    const float safeDepthRange = depthRange == 0.0f ? 1.0f : depthRange;
    const float yaw = yawDegrees * pi / 180.0f;
    const float pitch = pitchDegrees * pi / 180.0f;

    const float cosYaw = cosf(yaw);
    const float sinYaw = sinf(yaw);
    const float cosPitch = cosf(pitch);
    const float sinPitch = sinf(pitch);

    mProjectionMatrix = Matrix4f::Identity();
    mProjectionMatrix.at(0, 0) = cosYaw * 2.0f / safeViewWidth;
    mProjectionMatrix.at(0, 1) = -sinYaw * 2.0f / safeViewWidth;
    mProjectionMatrix.at(0, 2) = 0.0f;
    mProjectionMatrix.at(0, 3) = -(mProjectionMatrix.at(0, 0) * focus.x + mProjectionMatrix.at(0, 1) * focus.y + mProjectionMatrix.at(0, 2) * focus.z);

    mProjectionMatrix.at(1, 0) = sinPitch * sinYaw * 2.0f / safeViewHeight;
    mProjectionMatrix.at(1, 1) = sinPitch * cosYaw * 2.0f / safeViewHeight;
    mProjectionMatrix.at(1, 2) = cosPitch * 2.0f / safeViewHeight;
    mProjectionMatrix.at(1, 3) = -(mProjectionMatrix.at(1, 0) * focus.x + mProjectionMatrix.at(1, 1) * focus.y + mProjectionMatrix.at(1, 2) * focus.z);

    mProjectionMatrix.at(2, 0) = cosPitch * sinYaw * 2.0f / safeDepthRange;
    mProjectionMatrix.at(2, 1) = cosPitch * cosYaw * 2.0f / safeDepthRange;
    mProjectionMatrix.at(2, 2) = -sinPitch * 2.0f / safeDepthRange;
    mProjectionMatrix.at(2, 3) = -(mProjectionMatrix.at(2, 0) * focus.x + mProjectionMatrix.at(2, 1) * focus.y + mProjectionMatrix.at(2, 2) * focus.z);

    mProjectionMatrix.at(3, 0) = 0.0f;
    mProjectionMatrix.at(3, 1) = 0.0f;
    mProjectionMatrix.at(3, 2) = 0.0f;
    mProjectionMatrix.at(3, 3) = 1.0f;

    mWorldToCamTransformation = mProjectionMatrix;
    mPosition = focus;
}

void Camera::FPSCamera(const Vector3f &movement, const Vector3f &rotation) {
    const float x = movement.x;
    const float y = movement.y;
    const float z = movement.z;

    //Obter o eixo y atual da câmera
    Vector3f x_axis = mLookAt.CrossProduct(mUp).Normalized();

    //Atualizar as proriedades da camera
    mLookAt = Matrix4f::RotationMatrix(rotation.y, x_axis) * mLookAt;
    mUp = Matrix4f::RotationMatrix(rotation.y, x_axis) * mUp;
    //Atualizar as proriedades da camera
    mLookAt = Matrix4f::RotationMatrix(rotation.x, Vector3f(0, 1, 0)) * mLookAt;
    mUp = Matrix4f::RotationMatrix(rotation.x, Vector3f(0, 1, 0)) * mUp;

    //Atualiza a matrix de transformação da câmera
    mWorldToCamTransformation = Matrix4f::TranslationMatrix(mPosition * -1) * mWorldToCamTransformation;
    mWorldToCamTransformation = Matrix4f::RotationMatrix(rotation.y, x_axis) * mWorldToCamTransformation;
    mWorldToCamTransformation = Matrix4f::RotationMatrix(rotation.x, Vector3f(0, 1, 0)) * mWorldToCamTransformation;

    //Transçada a em seu sistema local
    mPosition = mPosition - x_axis.Normalized() * x - mUp.Normalized() * y - mLookAt.Normalized() * z;

    mWorldToCamTransformation = Matrix4f::TranslationMatrix(mPosition) * mWorldToCamTransformation;
}

void Camera::SetPerspectiveProjection() {
    Matrix4f projection = Matrix4f::Identity();
    projection.at(0, 0) = mZNear / mRight;
    projection.at(1, 1) = mZNear / mTop;
    projection.at(2, 2) = -(mZFar + mZNear) / (mZFar - mZNear);
    projection.at(2, 3) = -1;
    projection.at(3, 2) = -(2*mZFar * mZNear)/(mZFar - mZNear);
    projection.at(3, 3) = 0;

    mProjectionMatrix = projection.transpose();
}


void Camera::SetOrthographicProjection() {
    Matrix4f projection = Matrix4f::Identity();

    projection.at(0, 0) = 2.0f / (mRight - mLeft);
    projection.at(1, 1) = 2.0f / (mTop - mBottom);
    projection.at(2, 2) = -2.0f / (mZFar - mZNear);

    projection.at(0, 3) = -(mRight + mLeft) / (mRight - mLeft);
    projection.at(1, 3) = -(mTop + mBottom) / (mTop - mBottom);
    projection.at(2, 3) = -(mZFar + mZNear) / (mZFar - mZNear);
    projection.at(3, 3) = 1.0f;

    mProjectionMatrix = projection.transpose();
}

Camera::Camera(uint32_t width, uint32_t height, float fieldOfViewY, float zNear, float zFar)
    : mWidth(width)
    , mHeight(height)
    , mAspect(static_cast<float>(width) / static_cast<float>(height))
    , mFieldOfViewY(fieldOfViewY)
    , mTop(zNear * tanf(mFieldOfViewY/2 * (pi / 180.0f)))
    , mBottom(-mTop)
    , mZFar(zFar)
    , mZNear(zNear)
    , mFieldOfViewX(mFieldOfViewY * mAspect)
    , mRight(zNear * tanf(mFieldOfViewX/2 * (pi / 180.0f)))
    , mLeft(-mRight)
    , mTransformation(Matrix4f::Identity())
    , mViewMatrix(Matrix4f::Identity()){

    SetPerspectiveProjection();
    //mViewMatrix = Matrix4f::TranslationMatrix(focus);
    //mWorldToCamTransformation = mTransformation.Inverse();
    //mPosition = focus;

}

}// namespace unboxing_engine
