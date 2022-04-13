#include "Camera.h"

#include <cmath>

constexpr float pi = 3.1415926f;

void Camera::CastRayFromScreen(int scr_x, int scr_y, vector3D &point, vector3D &dir) const {
    //We calculate the ratio of screen coordinate by its respective axi size - i.g. scr_x/width
    //This ratio multiplies the near plane size in the same axi - i.g. 2*right.
    //The result is then subtracted by the plane origin to bring it to the world coordinate system - i.g. -left.
    float x =  (2.0f * mRight) * (static_cast<float>(scr_x)/mWidth) - mLeft;
    float y = (2.0f * mBottom) * (static_cast<float>(scr_y)/mHeight) + mTop;

    dir = (mWorldToCamTransformation * vector3D(x, y, mZNear)) - mPosition;
    point = mPosition;
}
void Camera::SetCamera(const vector3D &pos, float ang, const vector3D &axi, const vector3D &point) {
    mDirection = Matrix::rotationMatrix(ang, axi) * mDirection;
    mUp = Matrix::rotationMatrix(ang, axi) * mUp;

    //Atualiza a matrix de transformação da câmera
    mWorldToCamTransformation = Matrix::translationMatrix(mPosition * -1) * mWorldToCamTransformation;
    mWorldToCamTransformation = Matrix::rotationMatrix(ang, axi) * mWorldToCamTransformation;

    //Transçada a camera em seu sistema local
    mPosition = pos;

    mWorldToCamTransformation = Matrix::translationMatrix(pos) * mWorldToCamTransformation;

    mTransformation = mWorldToCamTransformation.inverse();
}

Camera::Camera(float width, float height, float FOV, float zNear, float zFar)
: mAspect(width/height)
, mFieldOfViewY(FOV)
, mTop(zNear * tanf(mFieldOfViewY * pi / 360.0))
, mBottom(-mTop)
, mZFar(zFar)
, mZNear(zNear)
, mFieldOfViewX(2 * atanf(tanf(mFieldOfViewY * 0.5) * mAspect))
, mRight(zNear * tanf(mFieldOfViewX * pi / 360.0))
, mLeft(-mRight) {}