#pragma once

#include "quaternion.h"
#include "matrix.h"

class Camera {
public:
    Camera(float width, float height, float FOV, float zNear, float zFar);
    ~Camera() = default;
    void CastRayFromScreen(int scr_x, int scr_y, vector3D &point, vector3D &dir) const;
    void SetCamera(const vector3D &pos, float ang, const vector3D &axi, const vector3D &point);

    //Opengl camera attributes
    quaternion mRotation = quaternion(0, vector3D(1, 0, 0));
    Matrix mTransformation = Matrix::identity(4);

    vector3D mDirection = vector3D(0, 0, -1);
    vector3D mPosition = vector3D(0, 0, 0);
    vector3D mUp = vector3D(0, 1, 0);
    Matrix mWorldToCamTransformation = Matrix::identity(4);

    float mWidth, mHeight;
    ///Screen aspect ratio
    float mAspect;
    ///Furthest coordinate away the screen
    float mZFar;
    ///Nearest coordinate close to the screen
    float mZNear;
    ///Vertical angle of visualization
    float mFieldOfViewY;
    ///Furthest coordinate to the top of the screen
    float mTop;
    ///Furthest coordinate to the bottom of the screen
    float mBottom;
    ///Horizontal angle of visualization
    float mFieldOfViewX;
    ///Furthest coordinate to the right of the screen
    float mRight;
    ///Furthest coordinate to the left of the screen
    float mLeft;
   

};
