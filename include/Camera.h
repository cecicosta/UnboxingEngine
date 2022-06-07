#pragma once

#include "Matrix.h"
#include "Quaternion.h"

#include <cstdint>

class Camera {
public:
    Camera(uint32_t width, uint32_t height, float FOV, float zNear, float zFar);
    ~Camera() = default;
    void CastRayFromScreen(int scr_x, int scr_y, Vector3f &point, Vector3f &dir) const;
    [[nodiscard]] Vector3f ScreenCoordinateToWorld(int scr_x, int scr_y, float zFar) const;
    void SetCamera(const Vector3f &pos, float ang, const Vector3f &axi, const Vector3f &point);
    ///Move the camera
    void FPSCamera(const Vector3f & movement, const Vector3f & rotation);

    //Opengl camera attributes
    Quaternion mRotation = Quaternion(0, Vector3f(1, 0, 0));
    Vector3f mDirection = Vector3f(0, 0, -1);
    Vector3f mPosition = Vector3f(0, 0, 0);
    Vector3f mUp = Vector3f(0, 1, 0);
    Matrix4f mTransformation;
    Matrix4f mWorldToCamTransformation;

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
