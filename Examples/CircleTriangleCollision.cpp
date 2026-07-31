#pragma once
#include "SceneCompositeLibrary/BasicSceneCompositeTypes.h"
#include "UnboxingEngine.h"
#include "algorithms/CollisionPrimitives.h"

using namespace unboxing_engine;

class CMouseTrackingCircle : public CSceneComposite
    , public UListener<core_events::IMouseInputEvent> {
public:
    CMouseTrackingCircle(CCore& engine, float radius) : mEngine(engine) {
        float angleIncrement = 360*(pi / 180) / 24.0;
        CMeshBuffer circleMesh(24);
        for (int i = 0; i < 24; ++i) {
            float x0 = radius * std::cos(angleIncrement*i);
            float y0 = radius * std::sin(angleIncrement*i);
            primitive_utils::AddVetex(circleMesh, Vector3f{x0, y0, 0});
        }
        mCircle = std::make_unique<CSimpleMeshWireFrame>(circleMesh);
        mCircle->SetParent(this);
        mEngine.RegisterSceneElement(*mCircle);
    }

    void Release() {
        mEngine.UnregisterSceneElement(*mCircle);
    }

    void OnMouseInputtEvent(const core_events::SCursor &cursor) override {
        if (!cursor.isButtonPressed) {
            return;
        }
        float currentZ = GetPosition().z;
        Vector3f outPoint, outDirection;
        auto camera = mEngine.GetCamera();
        camera.CastRayFromScreen(cursor.buttonPressedX, cursor.buttonPressedY, outPoint, outDirection);
        float x = outPoint.x + outDirection.x*(camera.GetPosition().z - currentZ);
        float y = outPoint.y + outDirection.y*(camera.GetPosition().z - currentZ);
        SetPosition(Vector3f{x, y, currentZ});

    }
private:
    CCore& mEngine;
    std::unique_ptr<CSimpleMeshWireFrame> mCircle;
};

class CCircleTriangleCollisionTest : public CSceneComposite, public UListener<core_events::IUpdateListener> {
public:
    CCircleTriangleCollisionTest(CCore& engine, const CMouseTrackingCircle& circle) : mCircle(circle) {


        mEdges.emplace_back(std::make_unique<CSimpleMeshWireFrame>(*primitive_utils::Lines(v1, v2)));
        mEdges.emplace_back(std::make_unique<CSimpleMeshWireFrame>(*primitive_utils::Lines(v2, v3)));
        mEdges.emplace_back(std::make_unique<CSimpleMeshWireFrame>(*primitive_utils::Lines(v3, v1)));

        for (auto&& edge: mEdges) {
            edge->SetParent(this);
            engine.RegisterSceneElement(*edge);
        }
    }

    void OnUpdate() override {
        SetCircleParameters(mCircle.GetPosition(), 0.05f);

        Vector3f v1t = GetTransformation() * v1;
        Vector3f v2t = GetTransformation() * v2;
        Vector3f v3t = GetTransformation() * v3;

        auto hit = collision_primitives::IntersectionSphereWithTriangle(mCircleCenter, mCircleRadius, v1, v2, v3);

        mEdges[0]->SetMaterial(greenMaterial());
        mEdges[1]->SetMaterial(greenMaterial());
        mEdges[2]->SetMaterial(greenMaterial());

        if (hit.hit) {
            bool intersectEdge = false;
            for (int i = 0; i < 3; ++i) {
                if (std::abs(hit.edges[i].distance) < mCircleRadius) {
                    if (hit.edges[i].v1 == v1 && hit.edges[i].v2 == v2) {
                        mEdges[0]->SetMaterial(blueMaterial());
                    } else if (hit.edges[i].v1 == v2 && hit.edges[i].v2 == v3) {
                        mEdges[1]->SetMaterial(blueMaterial());
                    } else if (hit.edges[i].v1 == v3 && hit.edges[i].v2 == v1) {
                        mEdges[2]->SetMaterial(blueMaterial());
                    }
                    intersectEdge = true;
                }
            }
            if (!intersectEdge) {
                mEdges[0]->SetMaterial(redMaterial());
                mEdges[1]->SetMaterial(redMaterial());
                mEdges[2]->SetMaterial(redMaterial());
            }
        }


    }

    void SetCircleParameters (const Vector3f& center, float radius) {
        mCircleRadius = radius;
        mCircleCenter = center;
    }
private:
    float mCircleRadius {0};
    Vector3f mCircleCenter;
    const CMouseTrackingCircle& mCircle;
    std::vector<std::unique_ptr<CSimpleMeshWireFrame>> mEdges;

    const Vector3f v1{0.5f, 0.5f, 0};
    const Vector3f v2{-0.5f, 0.5f, 0};
    const Vector3f v3{-0.5f, -0.5f, 0};
};

int main(int argc, char *argv[]) {
    unboxing_engine::CCore engine(640, 480, 32);
    Camera camera(1280, 720, 10, 1, 100);
    camera.SetOrthographicProjection();
    camera.SetPosition({0, 0, 10});
    engine.SetCamera(camera);
    engine.Start();

    float radius = 0.05f;
    CMouseTrackingCircle trackingCircle(engine, radius);
    trackingCircle.SetPosition({0, -0.6f, 0});
    engine.RegisterSceneElement(trackingCircle);

    CSimpleMeshWireFrame mark(*primitive_utils::Quad());
    mark.SetPosition({0, -0.8 - 0.05, 0});
    mark.SetScale({0.1, 0.1, 0.1});
    engine.RegisterSceneElement(mark);

    std::unique_ptr<CCircleTriangleCollisionTest> box = std::make_unique<CCircleTriangleCollisionTest>(engine, trackingCircle);
    engine.RegisterSceneElement(*box);

    engine.Run();

    engine.UnregisterSceneElement(*box);
    engine.UnregisterSceneElement(trackingCircle);
    trackingCircle.Release();
    engine.Release();
    return 0;
}