#include "systems/IRenderDebug.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>

namespace unboxing_engine::systems {
namespace {

class CStatisticsAccumulator {
public:
    CStatisticsAccumulator(const uint32_t width, const double nonZeroEpsilon)
        : mWidth(width)
        , mNonZeroEpsilon(std::max(0.0, nonZeroEpsilon)) {}

    void Add(const double value, const uint64_t pixelIndex) {
        ++mStatistics.sampleCount;
        if (std::isnan(value)) {
            ++mStatistics.nanCount;
            return;
        }
        if (std::isinf(value)) {
            ++mStatistics.infinityCount;
            return;
        }

        const auto location = Location(pixelIndex);
        ++mStatistics.finiteCount;
        mStatistics.sum += value;

        if (!mStatistics.hasFiniteValues || value < mStatistics.minimum) {
            mStatistics.minimum = value;
            mStatistics.minimumLocation = location;
        }
        if (!mStatistics.hasFiniteValues || value > mStatistics.maximum) {
            mStatistics.maximum = value;
            mStatistics.maximumLocation = location;
        }
        mStatistics.hasFiniteValues = true;

        if (std::abs(value) <= mNonZeroEpsilon) {
            ++mStatistics.zeroCount;
            return;
        }

        ++mStatistics.nonZeroCount;
        mNonZeroSum += value;
        if (value > 0.0) {
            ++mStatistics.positiveCount;
        } else {
            ++mStatistics.negativeCount;
        }

        if (!mStatistics.hasNonZeroValues) {
            mStatistics.nonZeroMinimum = value;
            mStatistics.nonZeroMaximum = value;
            mStatistics.nonZeroMinimumLocation = location;
            mStatistics.nonZeroMaximumLocation = location;
            mStatistics.nonZeroBoundsMinimum = location;
            mStatistics.nonZeroBoundsMaximum = location;
            mStatistics.hasNonZeroValues = true;
            return;
        }

        if (value < mStatistics.nonZeroMinimum) {
            mStatistics.nonZeroMinimum = value;
            mStatistics.nonZeroMinimumLocation = location;
        }
        if (value > mStatistics.nonZeroMaximum) {
            mStatistics.nonZeroMaximum = value;
            mStatistics.nonZeroMaximumLocation = location;
        }
        mStatistics.nonZeroBoundsMinimum.x = std::min(mStatistics.nonZeroBoundsMinimum.x, location.x);
        mStatistics.nonZeroBoundsMinimum.y = std::min(mStatistics.nonZeroBoundsMinimum.y, location.y);
        mStatistics.nonZeroBoundsMaximum.x = std::max(mStatistics.nonZeroBoundsMaximum.x, location.x);
        mStatistics.nonZeroBoundsMaximum.y = std::max(mStatistics.nonZeroBoundsMaximum.y, location.y);
        mStatistics.nonZeroBoundsMinimum.index =
            static_cast<uint64_t>(mStatistics.nonZeroBoundsMinimum.y) * mWidth +
            mStatistics.nonZeroBoundsMinimum.x;
        mStatistics.nonZeroBoundsMaximum.index =
            static_cast<uint64_t>(mStatistics.nonZeroBoundsMaximum.y) * mWidth +
            mStatistics.nonZeroBoundsMaximum.x;
    }

    [[nodiscard]] SValueStatistics Finish() {
        if (mStatistics.finiteCount != 0) {
            mStatistics.average = mStatistics.sum / static_cast<double>(mStatistics.finiteCount);
        }
        if (mStatistics.nonZeroCount != 0) {
            mStatistics.nonZeroAverage = mNonZeroSum / static_cast<double>(mStatistics.nonZeroCount);
        }
        if (mStatistics.sampleCount != 0) {
            mStatistics.nonZeroPercentage =
                100.0 * static_cast<double>(mStatistics.nonZeroCount) /
                static_cast<double>(mStatistics.sampleCount);
        }
        return mStatistics;
    }

private:
    [[nodiscard]] SPixelCoordinate Location(const uint64_t pixelIndex) const {
        return {
            static_cast<uint32_t>(pixelIndex % mWidth),
            static_cast<uint32_t>(pixelIndex / mWidth),
            pixelIndex
        };
    }

    uint32_t mWidth;
    double mNonZeroEpsilon;
    double mNonZeroSum = 0.0;
    SValueStatistics mStatistics;
};

const char* TextureFormatName(const ETextureFormat format) {
    switch (format) {
        case ETextureFormat::RGBA32F:
            return "RGBA32F";
        case ETextureFormat::RGBA8U:
        default:
            return "RGBA8U";
    }
}

std::string LocationString(const SPixelCoordinate& location) {
    std::ostringstream output;
    output << "(" << location.x << "," << location.y << ")"
           << "[" << location.index << "]";
    return output.str();
}

void AppendValueStatistics(
    std::ostringstream& output,
    const char* name,
    const SValueStatistics& statistics) {
    output << name << ":";
    if (statistics.hasFiniteValues) {
        output << " min=" << statistics.minimum << "@" << LocationString(statistics.minimumLocation)
               << " max=" << statistics.maximum << "@" << LocationString(statistics.maximumLocation)
               << " sum=" << statistics.sum
               << " avg=" << statistics.average;
    } else {
        output << " finite=none";
    }
    output << " nzPercent=" << statistics.nonZeroPercentage << "%";

    if (statistics.hasNonZeroValues) {
        output << " nzMin=" << statistics.nonZeroMinimum << "@"
               << LocationString(statistics.nonZeroMinimumLocation)
               << " nzMax=" << statistics.nonZeroMaximum << "@"
               << LocationString(statistics.nonZeroMaximumLocation)
               << " nzAvg=" << statistics.nonZeroAverage
               << " nzBounds=(" << statistics.nonZeroBoundsMinimum.x << ","
               << statistics.nonZeroBoundsMinimum.y << ")-("
               << statistics.nonZeroBoundsMaximum.x << ","
               << statistics.nonZeroBoundsMaximum.y << ")";
    } else {
        output << " nonzero=none";
    }

    output << " counts{samples=" << statistics.sampleCount
           << ",finite=" << statistics.finiteCount
           << ",zero=" << statistics.zeroCount
           << ",nonzero=" << statistics.nonZeroCount
           << ",positive=" << statistics.positiveCount
           << ",negative=" << statistics.negativeCount
           << ",nan=" << statistics.nanCount
           << ",inf=" << statistics.infinityCount
           << "}\n";
}

} // namespace

std::optional<STextureStatistics> CalculateTextureStatistics(
    const float* rgbaPixels,
    const size_t valueCount,
    const uint32_t width,
    const uint32_t height,
    const ETextureFormat format,
    const STextureInspectionOptions& options) {
    const uint64_t pixelCount = static_cast<uint64_t>(width) * height;
    if (!rgbaPixels || width == 0 || height == 0 || pixelCount > std::numeric_limits<size_t>::max() / 4 ||
        valueCount < static_cast<size_t>(pixelCount * 4)) {
        return std::nullopt;
    }

    std::array<CStatisticsAccumulator, 4> channelAccumulators = {
        CStatisticsAccumulator(width, options.nonZeroEpsilon),
        CStatisticsAccumulator(width, options.nonZeroEpsilon),
        CStatisticsAccumulator(width, options.nonZeroEpsilon),
        CStatisticsAccumulator(width, options.nonZeroEpsilon)
    };
    CStatisticsAccumulator magnitudeAccumulator(width, options.nonZeroEpsilon);

    for (uint64_t pixel = 0; pixel < pixelCount; ++pixel) {
        const size_t offset = static_cast<size_t>(pixel * 4);
        for (size_t channel = 0; channel < channelAccumulators.size(); ++channel) {
            channelAccumulators[channel].Add(rgbaPixels[offset + channel], pixel);
        }

        const double red = rgbaPixels[offset];
        const double green = rgbaPixels[offset + 1];
        const double blue = rgbaPixels[offset + 2];
        const double magnitude = std::sqrt(red * red + green * green + blue * blue);
        magnitudeAccumulator.Add(magnitude, pixel);
    }

    STextureStatistics result;
    result.width = width;
    result.height = height;
    result.format = format;
    result.nonZeroEpsilon = std::max(0.0, options.nonZeroEpsilon);
    for (size_t channel = 0; channel < channelAccumulators.size(); ++channel) {
        result.channels[channel] = channelAccumulators[channel].Finish();
    }
    result.rgbMagnitude = magnitudeAccumulator.Finish();
    return result;
}

std::string FormatTextureStatistics(
    const STextureStatistics& statistics,
    const std::string& label) {
    std::ostringstream output;
    output << std::setprecision(9);
    output << "Texture";
    if (!label.empty()) {
        output << " '" << label << "'";
    }
    output << ": " << statistics.width << "x" << statistics.height
           << " " << TextureFormatName(statistics.format)
           << " pixels=" << static_cast<uint64_t>(statistics.width) * statistics.height
           << " nonZeroEpsilon=" << statistics.nonZeroEpsilon << "\n";

    constexpr std::array<const char*, 4> channelNames = {"R", "G", "B", "A"};
    for (size_t channel = 0; channel < statistics.channels.size(); ++channel) {
        AppendValueStatistics(output, channelNames[channel], statistics.channels[channel]);
    }
    AppendValueStatistics(output, "RGB.length", statistics.rgbMagnitude);
    return output.str();
}

} // namespace unboxing_engine::systems
