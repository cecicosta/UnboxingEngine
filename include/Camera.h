#pragma once

#include "Matrix.h"
#include "Quaternion.h"
#include "internal_components/TransformComponent.h"

#include <arm_neon.h>
#include <cstdint>
namespace unboxing_engine {

class Camera {
public:
    /**
     *
     * @param width Screen width
     * @param height Screen height
     * @param fieldOfViewY Represents the complete field of view from top to bottom (Y direction)
     * @param zNear Distance from the 'eye' where the camera front projection plane is
     * @param zFar Farthest clipping plane
     */
    Camera(uint32_t width, uint32_t height, float fieldOfViewY, float zNear, float zFar);
    ~Camera() = default;
    void CastRayFromScreen(int scr_x, int scr_y, Vector3f &point, Vector3f &dir) const;
    [[nodiscard]] Vector3f ScreenCoordinateToWorld(int scr_x, int scr_y, float zFar) const;
    void SetOrthographicIsometric(float viewWidth, float viewHeight, float depthRange, const Vector3f &focus, float yawDegrees, float pitchDegrees);
    ///Move the camera
    void FPSCamera(const Vector3f &movement, const Vector3f &rotation);

    void SetPerspectiveProjection();
    void SetOrthographicProjection();

    void SetPosition(const Vector3f &position) {
        mIsDirty = true;
        mPosition = position;
    }

    Vector3f GetPosition() const {
        return mPosition;
    }

    void SetRotation(const Vector3f &axis, float angle) {
        mIsDirty = true;
        mRotation = Quaternion(angle, axis) * mRotation;
    }

    [[nodiscard]] const Matrix4f& GetProjectionMatrix() const {
        if (mIsDirty) {
            UpdateProjectionMatrix();
        }
        return mProjectionMatrix;
    }
    [[nodiscard]] const Matrix4f& GetViewMatrix() const {
        if (mIsDirty) {
            UpdateProjectionMatrix();
        }
        return mViewMatrix;
    }

    //Opengl camera attributes
    Vector3f mUp = Vector3f(0, 1, 0);
    Vector3f mLookAt = Vector3f(0, 0, 1);
    Matrix4f mTransformation;
    Matrix4f mProjectionMatrix;
    Matrix4f mWorldToCamTransformation;
    TransformComponent mTransformationComponent;

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
private:
    void UpdateProjectionMatrix() const {
        Matrix4f rotationMatrix = Matrix4f::RotationMatrix(mRotation).transpose();
        Matrix4f translationMatrix = Matrix4f::TranslationMatrix(mPosition*-1);
        mViewMatrix = rotationMatrix * translationMatrix;
        mIsDirty = false;
    }

    // Mutable variables for lazy update strategy
    mutable Matrix4f mViewMatrix;
    mutable bool mIsDirty;

    Quaternion mRotation = Quaternion(0, Vector3f(1, 0, 0));
    Vector3f mPosition = Vector3f(0, 0, 0);
};
}// namespace unboaxing_engine
