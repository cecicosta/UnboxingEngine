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

std::vector<SCornellMeshElement> makeCornellBoxMeshElements();

}  // namespace radiosity_demo
