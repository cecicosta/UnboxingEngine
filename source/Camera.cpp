#include "Camera.h"

#include <cmath>

constexpr float pi = 3.1415926f;

static inline void mat4x4_ortho( float* out, float left, float right, float bottom, float top, float znear, float zfar )
{
#define T(a, b) (a * 4 + b)

    out[T(0,0)] = 2.0f / (right - left);
    out[T(0,1)] = 0.0f;
    out[T(0,2)] = 0.0f;
    out[T(0,3)] = 0.0f;

    out[T(1,1)] = 2.0f / (top - bottom);
    out[T(1,0)] = 0.0f;
    out[T(1,2)] = 0.0f;
    out[T(1,3)] = 0.0f;

    out[T(2,2)] = -2.0f / (zfar - znear);
    out[T(2,0)] = 0.0f;
    out[T(2,1)] = 0.0f;
    out[T(2,3)] = 0.0f;

    out[T(3,0)] = -(right + left) / (right - left);
    out[T(3,1)] = -(top + bottom) / (top - bottom);
    out[T(3,2)] = -(zfar + znear) / (zfar - znear);
    out[T(3,3)] = 1.0f;

#undef T
}

void Camera::CastRayFromScreen(int scr_x, int scr_y, Vector3d &point, Vector3d &dir) const {
    //We calculate the ratio of mWindow coordinate by its respective axi size - i.g. scr_x/width
    //This ratio multiplies the near plane size in the same axi - i.g. 2*right.
    //The result is then subtracted by the plane origin to bring it to the world coordinate system - i.g. -left.
    float x =  (2.0f * mRight) * (static_cast<float>(scr_x)/mWidth) - mLeft;
    float y = (2.0f * mBottom) * (static_cast<float>(scr_y)/mHeight) + mTop;

    dir = (mWorldToCamTransformation * Vector3d(x, y, mZNear)) - mPosition;
    point = mPosition;
}

Vector3d Camera::ScreenCoordinateToWorld(int scr_x, int scr_y, float zFar) const {
    Vector3d point, direction;
    CastRayFromScreen(scr_x, scr_y, point, direction);
    return point + direction*zFar;
}

void Camera::SetCamera(const Vector3d &pos, float ang, const Vector3d &axi, const Vector3d &point) {
    mDirection = Matrix::rotationMatrix(ang, axi) * mDirection;
    mUp = Matrix::rotationMatrix(ang, axi) * mUp;

    //Run transformation matrix
    mWorldToCamTransformation = Matrix::translationMatrix(mPosition * -1) * mWorldToCamTransformation;
    mWorldToCamTransformation = Matrix::rotationMatrix(ang, axi) * mWorldToCamTransformation;

    //Translate camera in the local system
    mPosition = pos;

    mWorldToCamTransformation = Matrix::translationMatrix(pos) * mWorldToCamTransformation;

    mTransformation = mWorldToCamTransformation.inverse();
}

void Camera::FPSCamera(const Vector3d & movement, const Vector3d & rotation) {
    const float x = movement.x;
    const float y = movement.y;
    const float z = movement.z;

    //Obter o eixo y atual da câmera
    Vector3d x_axis = mDirection.CrossProduct(mUp).Normalized();

    //Atualizar as proriedades da camera
    mDirection = Matrix::rotationMatrix(rotation.y, x_axis) * mDirection;
    mUp = Matrix::rotationMatrix(rotation.y, x_axis) * mUp;
    //Atualizar as proriedades da camera
    mDirection = Matrix::rotationMatrix(rotation.x, Vector3d(0, 1, 0)) * mDirection;
    mUp = Matrix::rotationMatrix(rotation.x, Vector3d(0, 1, 0)) * mUp;

    //Atualiza a matrix de transformação da câmera
    mWorldToCamTransformation = Matrix::translationMatrix(mPosition * -1) * mWorldToCamTransformation;
    mWorldToCamTransformation = Matrix::rotationMatrix(rotation.y, x_axis) * mWorldToCamTransformation;
    mWorldToCamTransformation = Matrix::rotationMatrix(rotation.x, Vector3d(0, 1, 0)) * mWorldToCamTransformation;

    //Transçada a em seu sistema local
    mPosition = mPosition - x_axis.Normalized() * x - mUp.Normalized() * y - mDirection.Normalized() * z;

    mWorldToCamTransformation = Matrix::translationMatrix(mPosition) * mWorldToCamTransformation;

    mTransformation = mWorldToCamTransformation.inverse();
}

Camera::Camera(uint32_t width, uint32_t height, float FOV, float zNear, float zFar)
: mWidth(width), mHeight(height)
, mAspect(static_cast<float>(width)/static_cast<float>(height))
, mFieldOfViewY(FOV)
, mTop(zNear * tanf(mFieldOfViewY * pi / 360.0f))
, mBottom(-mTop)
, mZFar(zFar)
, mZNear(zNear)
, mFieldOfViewX(2 * atanf(tanf(mFieldOfViewY * 0.5f) * mAspect))
, mRight(zNear * tanf(mFieldOfViewX * pi / 360.0f))
, mLeft(-mRight) {
    //mat4x4_ortho(mTransformation.getMatrixGL(),mLeft, mRight, mBottom, mTop, zNear, zFar );
}