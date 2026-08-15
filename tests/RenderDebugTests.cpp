#include "systems/IRenderDebug.h"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <string>
#include <vector>

namespace {

using unboxing_engine::systems::CalculateTextureStatistics;
using unboxing_engine::systems::ETextureFormat;
using unboxing_engine::systems::FormatTextureStatistics;
using unboxing_engine::systems::STextureInspectionOptions;

TEST(RenderDebugTest, CalculatesChannelAndVectorStatistics) {
    const std::vector<float> pixels = {
        0.0f,  1.0f, -2.0f, std::numeric_limits<float>::quiet_NaN(),
        2.0f,  0.0f,  4.0f, std::numeric_limits<float>::infinity(),
       -1.0f,  3.0f,  0.0f, -std::numeric_limits<float>::infinity(),
        5.0f, -1.0f,  2.0f, 1.0f
    };

    const auto result = CalculateTextureStatistics(
        pixels.data(), pixels.size(), 2, 2, ETextureFormat::RGBA32F);

    ASSERT_TRUE(result.has_value());
    const auto& red = result->channels[0];
    EXPECT_DOUBLE_EQ(red.minimum, -1.0);
    EXPECT_EQ(red.minimumLocation.x, 0u);
    EXPECT_EQ(red.minimumLocation.y, 1u);
    EXPECT_EQ(red.minimumLocation.index, 2u);
    EXPECT_DOUBLE_EQ(red.maximum, 5.0);
    EXPECT_EQ(red.maximumLocation.x, 1u);
    EXPECT_EQ(red.maximumLocation.y, 1u);
    EXPECT_EQ(red.maximumLocation.index, 3u);
    EXPECT_DOUBLE_EQ(red.sum, 6.0);
    EXPECT_DOUBLE_EQ(red.average, 1.5);
    EXPECT_DOUBLE_EQ(red.nonZeroAverage, 2.0);
    EXPECT_EQ(red.zeroCount, 1u);
    EXPECT_EQ(red.nonZeroCount, 3u);
    EXPECT_DOUBLE_EQ(red.nonZeroPercentage, 75.0);
    EXPECT_EQ(red.positiveCount, 2u);
    EXPECT_EQ(red.negativeCount, 1u);

    const auto& alpha = result->channels[3];
    EXPECT_EQ(alpha.finiteCount, 1u);
    EXPECT_EQ(alpha.nanCount, 1u);
    EXPECT_EQ(alpha.infinityCount, 2u);
    EXPECT_DOUBLE_EQ(alpha.average, 1.0);

    const auto& magnitude = result->rgbMagnitude;
    EXPECT_NEAR(magnitude.minimum, std::sqrt(5.0), 1e-6);
    EXPECT_NEAR(magnitude.maximum, std::sqrt(30.0), 1e-6);
    EXPECT_EQ(magnitude.maximumLocation.x, 1u);
    EXPECT_EQ(magnitude.maximumLocation.y, 1u);
}

TEST(RenderDebugTest, AppliesNonZeroEpsilonToCountsAndBounds) {
    const std::vector<float> pixels = {
        0.0001f, 0.0f, 0.0f, 0.0f,
       -0.01f,   0.0f, 0.0f, 0.0f
    };
    STextureInspectionOptions options;
    options.nonZeroEpsilon = 0.001;

    const auto result = CalculateTextureStatistics(
        pixels.data(), pixels.size(), 2, 1, ETextureFormat::RGBA32F, options);

    ASSERT_TRUE(result.has_value());
    const auto& red = result->channels[0];
    EXPECT_EQ(red.zeroCount, 1u);
    EXPECT_EQ(red.nonZeroCount, 1u);
    EXPECT_EQ(red.positiveCount, 0u);
    EXPECT_EQ(red.negativeCount, 1u);
    EXPECT_EQ(red.nonZeroBoundsMinimum.x, 1u);
    EXPECT_EQ(red.nonZeroBoundsMaximum.x, 1u);
}

TEST(RenderDebugTest, RejectsIncompletePixelData) {
    const std::vector<float> pixels(7, 0.0f);

    EXPECT_FALSE(CalculateTextureStatistics(
        pixels.data(), pixels.size(), 2, 1, ETextureFormat::RGBA32F).has_value());
}

TEST(RenderDebugTest, FormatsStableMeaningfulOutput) {
    const std::vector<float> pixels = {1.0f, 0.0f, -2.0f, 1.0f};
    const auto statistics = CalculateTextureStatistics(
        pixels.data(), pixels.size(), 1, 1, ETextureFormat::RGBA32F);
    ASSERT_TRUE(statistics.has_value());

    const std::string output = FormatTextureStatistics(*statistics, "test texture");

    EXPECT_NE(output.find("Texture 'test texture': 1x1 RGBA32F"), std::string::npos);
    EXPECT_NE(output.find("R: min=1@(0,0)[0]"), std::string::npos);
    EXPECT_NE(output.find("B: min=-2@(0,0)[0]"), std::string::npos);
    EXPECT_NE(output.find("RGB.length:"), std::string::npos);
}

} // namespace
