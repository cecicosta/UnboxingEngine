#include "Camera.h"

#include <cmath>

constexpr float pi = 3.1415926f;

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

void Camera::CastRayFromScreen(int scr_x, int scr_y, Vector3f &point, Vector3f &dir) const {
    //We calculate the ratio of mWindow coordinate by its respective axi size - i.g. scr_x/width
    //This ratio multiplies the near plane size in the same axi - i.g. 2*right.
    //The result is then subtracted by the plane origin to bring it to the world coordinate system - i.g. -left.
    float x = (2.0f * mRight) * (static_cast<float>(scr_x) / mWidth) - mLeft;
    float y = (2.0f * mBottom) * (static_cast<float>(scr_y) / mHeight) + mTop;

    dir = (mWorldToCamTransformation * Vector3f(x, y, mZNear)) - mPosition;
    point = mPosition;
}

Vector3f Camera::ScreenCoordinateToWorld(int scr_x, int scr_y, float zFar) const {
    Vector3f point, direction;
    CastRayFromScreen(scr_x, scr_y, point, direction);
    return point + direction * zFar;
}

void Camera::SetCamera(const Vector3f &pos, float ang, const Vector3f &axi, const Vector3f &point) {
    mDirection = Matrix4f::RotationMatrix(ang, axi) * mDirection;
    mUp = Matrix4f::RotationMatrix(ang, axi) * mUp;

    //Run transformation matrix
    mWorldToCamTransformation = Matrix4f::TranslationMatrix((mPosition * -1)) * mWorldToCamTransformation;
    mWorldToCamTransformation = Matrix4f::RotationMatrix(ang, axi) * mWorldToCamTransformation;

    //Translate camera in the local system
    mPosition = pos;

    mWorldToCamTransformation = Matrix4f::TranslationMatrix(pos) * mWorldToCamTransformation;

    mTransformation = mWorldToCamTransformation.Inverse();
}

void Camera::FPSCamera(const Vector3f &movement, const Vector3f &rotation) {
    const float x = movement.x;
    const float y = movement.y;
    const float z = movement.z;

    //Obter o eixo y atual da câmera
    Vector3f x_axis = mDirection.CrossProduct(mUp).Normalized();

    //Atualizar as proriedades da camera
    mDirection = Matrix4f::RotationMatrix(rotation.y, x_axis) * mDirection;
    mUp = Matrix4f::RotationMatrix(rotation.y, x_axis) * mUp;
    //Atualizar as proriedades da camera
    mDirection = Matrix4f::RotationMatrix(rotation.x, Vector3f(0, 1, 0)) * mDirection;
    mUp = Matrix4f::RotationMatrix(rotation.x, Vector3f(0, 1, 0)) * mUp;

    //Atualiza a matrix de transformação da câmera
    mWorldToCamTransformation = Matrix4f::TranslationMatrix(mPosition * -1) * mWorldToCamTransformation;
    mWorldToCamTransformation = Matrix4f::RotationMatrix(rotation.y, x_axis) * mWorldToCamTransformation;
    mWorldToCamTransformation = Matrix4f::RotationMatrix(rotation.x, Vector3f(0, 1, 0)) * mWorldToCamTransformation;

    //Transçada a em seu sistema local
    mPosition = mPosition - x_axis.Normalized() * x - mUp.Normalized() * y - mDirection.Normalized() * z;

    mWorldToCamTransformation = Matrix4f::TranslationMatrix(mPosition) * mWorldToCamTransformation;

    mTransformation = mWorldToCamTransformation.Inverse();
}

Camera::Camera(uint32_t width, uint32_t height, float FOV, float zNear, float zFar)
    : mWidth(width)
    , mHeight(height)
    , mAspect(static_cast<float>(width) / static_cast<float>(height))
    , mFieldOfViewY(FOV)
    , mTop(zNear * tanf(mFieldOfViewY * pi / 360.0f))
    , mBottom(-mTop)
    , mZFar(zFar)
    , mZNear(zNear)
    , mFieldOfViewX(2 * atanf(tanf(mFieldOfViewY * 0.5f) * mAspect))
    , mRight(zNear * tanf(mFieldOfViewX * pi / 360.0f))
    , mLeft(-mRight)
    , mTransformation(Matrix4f::Identity()) {}