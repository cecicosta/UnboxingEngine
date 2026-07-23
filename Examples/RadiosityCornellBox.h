#pragma once

#include "MeshBuffer.h"

#include <memory>
#include <string>
#include <vector>

namespace radiosity_demo {

struct SRgb {
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
};

struct SCornellMeshElement {
    std::string name;
    std::unique_ptr<unboxing_engine::CMeshBuffer> mesh;
    SRgb reflectance;
    SRgb emissivity;
};

struct SCornellBoxSubdivision {
    unsigned int floorColumns = 8;
    unsigned int floorRows = 8;
    unsigned int wallColumns = 8;
    unsigned int wallRows = 6;
    unsigned int ceilingColumns = 8;
    unsigned int ceilingRows = 8;
    unsigned int lightColumns = 3;
    unsigned int lightRows = 3;
};

struct SCornellTorusConfig {
    bool enabled = false;
    float majorRadius = 0.52f;
    float minorRadius = 0.20f;
    unsigned int majorSegments = 24;
    unsigned int minorSegments = 8;
    Vector3f center = Vector3f(0.15f, -0.10f, 1.35f);
    SRgb reflectance = {0.16f, 0.25f, 0.86f};
    SRgb emissivity;
};

struct SCornellBoxConfig {
    SCornellBoxSubdivision subdivision;
    SCornellTorusConfig torus;
    float lightEmissionScale = 2.0f;
};

std::vector<SCornellMeshElement> makeCornellBoxMeshElements();
std::vector<SCornellMeshElement> makeCornellBoxMeshElements(const SCornellBoxConfig &config);

}  // namespace radiosity_demo
