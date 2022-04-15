#pragma once

#include "quaternion.h"
#include "matrix.h"

class Camera {
public:
    Camera(uint32_t width, uint32_t height, float FOV, float zNear, float zFar);
    ~Camera() = default;
    void CastRayFromScreen(int scr_x, int scr_y, vector3D &point, vector3D &dir) const;
    vector3D ScreenCoordinateToWorld(int scr_x, int scr_y, float zFar) const;
    void SetCamera(const vector3D &pos, float ang, const vector3D &axi, const vector3D &point);

    //Opengl camera attributes
    quaternion mRotation = quaternion(0, vector3D(1, 0, 0));
    Matrix mTransformation = Matrix::identity(4);
    vector3D mDirection = vector3D(0, 0, -1);
    vector3D mPosition = vector3D(0, 0, 0);
    vector3D mUp = vector3D(0, 1, 0);
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
