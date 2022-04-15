#include "Camera.h"

#include <cmath>

constexpr float pi = 3.1415926f;

void Camera::CastRayFromScreen(int scr_x, int scr_y, vector3D &point, vector3D &dir) const {
    //We calculate the ratio of mWindow coordinate by its respective axi size - i.g. scr_x/width
    //This ratio multiplies the near plane size in the same axi - i.g. 2*right.
    //The result is then subtracted by the plane origin to bring it to the world coordinate system - i.g. -left.
    float x =  (2.0f * mRight) * (static_cast<float>(scr_x)/mWidth) - mLeft;
    float y = (2.0f * mBottom) * (static_cast<float>(scr_y)/mHeight) + mTop;

    dir = (mWorldToCamTransformation * vector3D(x, y, mZNear)) - mPosition;
    point = mPosition;
}

vector3D Camera::ScreenCoordinateToWorld(int scr_x, int scr_y, float zFar) const {
    vector3D point, direction;
    CastRayFromScreen(scr_x, scr_y, point, direction);
    return point + direction*zFar;
}

void Camera::SetCamera(const vector3D &pos, float ang, const vector3D &axi, const vector3D &point) {
    mDirection = Matrix::rotationMatrix(ang, axi) * mDirection;
    mUp = Matrix::rotationMatrix(ang, axi) * mUp;

    //Update transformation matrix
    mWorldToCamTransformation = Matrix::translationMatrix(mPosition * -1) * mWorldToCamTransformation;
    mWorldToCamTransformation = Matrix::rotationMatrix(ang, axi) * mWorldToCamTransformation;

    //Translate camera in the local system
    mPosition = pos;

    mWorldToCamTransformation = Matrix::translationMatrix(pos) * mWorldToCamTransformation;

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
, mLeft(-mRight) {}