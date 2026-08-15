#pragma once

#include "systems/IRenderSystem.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

namespace unboxing_engine::systems {

struct SPixelCoordinate {
    // Texture coordinates use OpenGL's bottom-left origin.
    uint32_t x = 0;
    uint32_t y = 0;
    uint64_t index = 0;
};

struct SValueStatistics {
    bool hasFiniteValues = false;
    bool hasNonZeroValues = false;

    double minimum = 0.0;
    double maximum = 0.0;
    double nonZeroMinimum = 0.0;
    double nonZeroMaximum = 0.0;
    double sum = 0.0;
    // Averages include finite values only; NaN and infinity are counted separately.
    double average = 0.0;
    double nonZeroAverage = 0.0;
    double nonZeroPercentage = 0.0;

    SPixelCoordinate minimumLocation;
    SPixelCoordinate maximumLocation;
    SPixelCoordinate nonZeroMinimumLocation;
    SPixelCoordinate nonZeroMaximumLocation;
    SPixelCoordinate nonZeroBoundsMinimum;
    SPixelCoordinate nonZeroBoundsMaximum;

    uint64_t sampleCount = 0;
    uint64_t finiteCount = 0;
    uint64_t zeroCount = 0;
    uint64_t nonZeroCount = 0;
    uint64_t positiveCount = 0;
    uint64_t negativeCount = 0;
    uint64_t nanCount = 0;
    uint64_t infinityCount = 0;
};

struct STextureStatistics {
    uint32_t width = 0;
    uint32_t height = 0;
    ETextureFormat format = ETextureFormat::RGBA8U;
    double nonZeroEpsilon = 0.0;
    std::array<SValueStatistics, 4> channels;
    SValueStatistics rgbMagnitude;
};

struct STextureInspectionOptions {
    // Values whose absolute magnitude is at most this threshold count as zero.
    double nonZeroEpsilon = 0.0;
};

class IRenderDebug {
public:
    virtual ~IRenderDebug() = default;

    // Texture readback is synchronous and may stall until pending GPU work completes.
    [[nodiscard]] virtual std::optional<STextureStatistics> InspectTexture(
        const STextureHandle& texture,
        const STextureInspectionOptions& options = {}) const = 0;

    virtual bool PrintTextureStatistics(
        const STextureHandle& texture,
        const std::string& label = {},
        const STextureInspectionOptions& options = {}) const = 0;
};

[[nodiscard]] std::optional<STextureStatistics> CalculateTextureStatistics(
    const float* rgbaPixels,
    size_t valueCount,
    uint32_t width,
    uint32_t height,
    ETextureFormat format,
    const STextureInspectionOptions& options = {});

[[nodiscard]] std::string FormatTextureStatistics(
    const STextureStatistics& statistics,
    const std::string& label = {});

} // namespace unboxing_engine::systems
