#pragma once
#include "SceneCompositeLibrary/BasicSceneCompositeTypes.h"
#include "UnboxingEngine.h"
#include "algorithms/CollisionPrimitives.h"

constexpr float E0 = 8.854e-12; // F/m [Farads per meter] 1C charge per 1V potential difference, every 1 metter


using namespace unboxing_engine;



/**
 * Calculates the energy differential by distance as the definition of force.
 * F = dE/dx
 */
float EnergyDifferential(const float dx) {

}
float Force(const float delta_x) {

}

// We want initially simulate a time independent system of a single hydrogen atom in the ground state
// I. Follow the definition for the nucleus potential
// II. We do not intend to make time evolution, so render the probability space of the particles
// III. For this, we will need to render the electron on its probability space
// IV. The radius of a particle must be dynamic. The geometry might be centered on its peak of probability,
// but it extends up until its current state allows. The region shift might occur, as wave packet translation.
// The influence of external fields must be passed as parameter for the particle shader.

/**
 * This class models fields which affects particles across space
 */
struct ESpaceField {
    std::function<float(float radius, float azimuth, float bearing)> field;
    Vector3f position;
    // Distance from the scenter from which the field calculation is valid (planet surface, black hole event horizon, nucleus radius).
    float horizon;
};


class CMouseTrackingSphere : public CustomShaderComposite, public UListener<core_events::IMouseInputEvent> {
public:
    CMouseTrackingSphere(CCore& engine, float radius) : CustomShaderComposite(*primitive_utils::Sphere(radius)), mEngine(engine){
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
};


int main(int argc, char *argv[]) {
    unboxing_engine::CCore engine(640, 480, 32);
    Camera camera(720, 720, 60, 1, 200);
    camera.SetPerspectiveProjection();
    camera.SetPosition({0, 0, 100});
    engine.SetCamera(camera);
    engine.Start();


    std::function<float(uint32_t Z, const float r)> CoulombPotential = [](uint32_t Z, const float r) {
        return (Z*std::exp(2))/(4*PI*E0*r);
    };

    uint32_t Z = 1;
    const float INF = (Z*std::exp(2))/(4*PI*E0*kEpsilon);

    for (float i = 0.1; i <= INF; i=i+INF/10.0) {
        const float fadeFactor = std::exp(2)/(4*PI*E0*i);
        std::cout << "fadeFactor = " << fadeFactor << std::endl;
    }

    float radius = 40;
    CMouseTrackingSphere trackingCircle(engine, radius);
    trackingCircle.SetPosition({0, 0, 0});
    engine.RegisterSceneElement(trackingCircle);

    CSimpleMeshWireFrame test(*primitive_utils::Cube());
    test.SetMaterial(redMaterial());
    engine.RegisterSceneElement(test);

    engine.Run();

    engine.UnregisterSceneElement(trackingCircle);
    engine.UnregisterSceneElement(test);
    engine.Release();
    return 0;
}