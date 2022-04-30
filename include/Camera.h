#pragma once

#include "quaternion.h"
#include "matrix.h"

#include <cstdint>

class Camera {
public:
    Camera(uint32_t width, uint32_t height, float FOV, float zNear, float zFar);
    ~Camera() = default;
    void CastRayFromScreen(int scr_x, int scr_y, Vector3d &point, Vector3d &dir) const;
    Vector3d ScreenCoordinateToWorld(int scr_x, int scr_y, float zFar) const;
    void SetCamera(const Vector3d &pos, float ang, const Vector3d &axi, const Vector3d &point);
    ///Move the camera
    void FPSCamera(const Vector3d & movement, const Vector3d & rotation);

    //Opengl camera attributes
    quaternion mRotation = quaternion(0, Vector3d(1, 0, 0));
    Matrix mTransformation = Matrix::identity(4);
    Vector3d mDirection = Vector3d(0, 0, -1);
    Vector3d mPosition = Vector3d(0, 0, 0);
    Vector3d mUp = Vector3d(0, 1, 0);
    Matrix mWorldToCamTransformation = Matrix::identity(4);

    unsigned int mWidth, mHeight;
    ///Screen aspect ratio
    float mAspect;
    ///Furthest coordinate away the mWindow
    float mZFar;
    ///Nearest coordinate close to the mWindow
    float mZNear;
    ///Vertical angle of visualization
    float mFieldOfViewY;
    ///Furthest coordinate to the top of the mWindow
    float mTop;
    ///Furthest coordinate to the bottom of the mWindow
    float mBottom;
    ///Horizontal angle of visualization
    float mFieldOfViewX;
    ///Furthest coordinate to the right of the mWindow
    float mRight;
    ///Furthest coordinate to the left of the mWindow
    float mLeft;
};
