#pragma once

#include "Matrix.h"
#include "Quaternion.h"

#include <cstdint>

class Camera {
public:
    Camera(uint32_t width, uint32_t height, float FOV, float zNear, float zFar);
    ~Camera() = default;
    void CastRayFromScreen(int scr_x, int scr_y, Vector3Df &point, Vector3Df &dir) const;
    [[nodiscard]] Vector3Df ScreenCoordinateToWorld(int scr_x, int scr_y, float zFar) const;
    void SetCamera(const Vector3Df &pos, float ang, const Vector3Df &axi, const Vector3Df &point);
    ///Move the camera
    void FPSCamera(const Vector3Df & movement, const Vector3Df & rotation);

    //Opengl camera attributes
    Quaternion mRotation = Quaternion(0, Vector3Df(1, 0, 0));
    Vector3Df mDirection = Vector3Df(0, 0, -1);
    Vector3Df mPosition = Vector3Df(0, 0, 0);
    Vector3Df mUp = Vector3Df(0, 1, 0);
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
