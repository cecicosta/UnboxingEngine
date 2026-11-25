#pragma once

#include "RadiosityCornellBox.h"

#include <array>
#include <cstddef>

namespace radiosity_demo {

inline constexpr float kEpsilon = 0.00001f;

struct SViewedPatch {
    int id = -1;
    float formFactor = 0.0f;
};

struct SRadiosityPatch {
    int id = -1;
    std::string name;
    std::array<Vector3f, 3> vertex;
    Vector3f centroid;
    Vector3f normal;
    float area = 0.0f;
    SRgb emissivity;
    SRgb radiosity;
    SRgb reflectance;
    std::vector<SViewedPatch> patches;
    std::size_t elementIndex = 0;
};

struct SRadiositySummary {
    int patchCount = 0;
    int nonZeroFormFactors = 0;
    int iterations = 0;
    float maxFormFactor = 0.0f;
    float maxRowSum = 0.0f;
    float maxDelta = 0.0f;
};

SRgb rgb(float r, float g, float b);
float maxChannel(const SRgb &color);
float clamp01(float value);
SRgb toneMap(const SRgb &color, float exposure = 0.55f);

SRadiosityPatch makePatchPorted(
    int id,
    const std::string &name,
    const Vector3f &v0,
    const Vector3f &v1,
    const Vector3f &v2,
    const SRgb &reflectance,
    const SRgb &emissivity = {});
bool isVisiblePorted(const SRadiosityPatch &receiver, const SRadiosityPatch &source);
SRadiositySummary calculateFormFactorScenePorted(std::vector<SRadiosityPatch> &patches);
void gaussSeidelRadiosityPorted(
    std::vector<SRadiosityPatch> &patches,
    SRadiositySummary &summary,
    int maxIterations = 80,
    float tolerance = 0.0001f);
std::vector<SRadiosityPatch> makePatchesFromMeshPorted(const std::vector<SCornellMeshElement> &meshElements);
std::vector<SRadiosityPatch> makeCornellBoxScenePorted();

} // namespace radiosity_demo
