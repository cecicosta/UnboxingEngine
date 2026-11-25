#include "RadiositySolver.h"

#include <emscripten/bind.h>

#include <algorithm>
#include <cmath>

namespace {

using namespace radiosity_demo;

struct Settings {
    SRgb left;
    SRgb back;
    SRgb right;
    float torusScale = 1.0f;
    float light = 3.0f;
};

struct Result {
    int patches = 0;
    int formFactors = 0;
    int iterations = 0;
    bool rebuiltGeometry = false;
};

enum class Surface { Other, Left, Back, Right, Light };

Surface surfaceFor(const std::string &name) {
    if (name == "left wall") return Surface::Left;
    if (name == "back wall") return Surface::Back;
    if (name == "right wall") return Surface::Right;
    if (name == "emissive ceiling patch") return Surface::Light;
    return Surface::Other;
}

SRgb reflectance(const SRgb &color) {
    return {clamp01(color.r), clamp01(color.g), clamp01(color.b)};
}

struct Lab {
    std::vector<SRadiosityPatch> patches;
    SRadiositySummary factors;
    std::vector<Surface> surfaces;
    std::vector<SRgb> baseEmission;
    std::vector<float> positions;
    std::vector<float> colors;
    float previousScale = 0.0f;

    void rebuild(float scale) {
        SCornellBoxConfig config;
        config.torus.majorRadius *= scale;
        config.torus.minorRadius *= scale;
        config.lightEmissionScale = 1.0f;
        const auto elements = makeCornellBoxMeshElements(config);
        patches = makePatchesFromMeshPorted(elements);
        factors = calculateFormFactorScenePorted(patches);
        previousScale = scale;

        surfaces.clear();
        baseEmission.clear();
        for (const auto &element : elements) {
            surfaces.push_back(surfaceFor(element.name));
            baseEmission.push_back(element.emissivity);
        }
        positions.clear();
        positions.reserve(patches.size() * 9);
        for (const auto &patch : patches) {
            for (const auto &vertex : patch.vertex) {
                positions.insert(positions.end(), {vertex.x, vertex.y, vertex.z});
            }
        }
        colors.resize(positions.size());
    }

    Result solve(const Settings &settings) {
        const float scale = std::isfinite(settings.torusScale)
            ? std::clamp(settings.torusScale, 0.5f, 1.8f) : 1.0f;
        const float light = std::isfinite(settings.light)
            ? std::clamp(settings.light, 0.0f, 5.0f) : 3.0f;
        const bool rebuilt = scale != previousScale;
        if (rebuilt) {
            rebuild(scale);
        }

        for (auto &patch : patches) {
            switch (surfaces[patch.elementIndex]) {
            case Surface::Left: patch.reflectance = reflectance(settings.left); break;
            case Surface::Back: patch.reflectance = reflectance(settings.back); break;
            case Surface::Right: patch.reflectance = reflectance(settings.right); break;
            case Surface::Light: {
                const auto &base = baseEmission[patch.elementIndex];
                patch.emissivity = {base.r * light, base.g * light, base.b * light};
                break;
            }
            case Surface::Other: break;
            }
        }

        auto summary = factors;
        gaussSeidelRadiosityPorted(patches, summary);
        for (std::size_t i = 0; i < patches.size(); ++i) {
            const auto color = toneMap(patches[i].radiosity);
            for (std::size_t vertex = 0; vertex < 3; ++vertex) {
                const std::size_t offset = i * 9 + vertex * 3;
                colors[offset] = color.r;
                colors[offset + 1] = color.g;
                colors[offset + 2] = color.b;
            }
        }
        return {summary.patchCount, summary.nonZeroFormFactors, summary.iterations, rebuilt};
    }
};

Lab &lab() {
    static Lab instance;
    return instance;
}

} // namespace

EMSCRIPTEN_BINDINGS(radiosity_lab) {
    using namespace emscripten;
    value_object<SRgb>("Rgb")
        .field("r", &SRgb::r).field("g", &SRgb::g).field("b", &SRgb::b);
    value_object<Settings>("Settings")
        .field("left", &Settings::left)
        .field("back", &Settings::back)
        .field("right", &Settings::right)
        .field("torusScale", &Settings::torusScale)
        .field("light", &Settings::light);
    value_object<Result>("Result")
        .field("patches", &Result::patches)
        .field("formFactors", &Result::formFactors)
        .field("iterations", &Result::iterations)
        .field("rebuiltGeometry", &Result::rebuiltGeometry);

    function("solve", +[](const Settings &settings) { return lab().solve(settings); });
    function("positions", +[] {
        const auto &positions = lab().positions;
        return val(typed_memory_view(positions.size(), positions.data()));
    });
    function("colors", +[] {
        const auto &colors = lab().colors;
        return val(typed_memory_view(colors.size(), colors.data()));
    });
}
