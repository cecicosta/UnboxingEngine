#include "systems/IRenderDebug.h"

#include <gtest/gtest.h>

#include <cmath>
#include <limits>
#include <string>
#include <utility>
#include <vector>

namespace {

using unboxing_engine::systems::CalculateTextureStatistics;
using unboxing_engine::systems::CompareTextureSnapshots;
using unboxing_engine::systems::ETextureFormat;
using unboxing_engine::systems::FormatTextureDifferenceStatistics;
using unboxing_engine::systems::FormatTextureStatistics;
using unboxing_engine::systems::STextureInspectionOptions;
using unboxing_engine::systems::STextureSnapshot;

STextureSnapshot MakeSnapshot(
    const uint32_t width,
    const uint32_t height,
    std::vector<float> pixels) {
    STextureSnapshot snapshot;
    snapshot.width = width;
    snapshot.height = height;
    snapshot.format = ETextureFormat::RGBA32F;
    snapshot.rgbaPixels = std::move(pixels);
    const auto statistics = CalculateTextureStatistics(
        snapshot.rgbaPixels.data(),
        snapshot.rgbaPixels.size(),
        width,
        height,
        snapshot.format);
    if (statistics) {
        snapshot.statistics = *statistics;
    }
    return snapshot;
}

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

TEST(RenderDebugTest, ComparesSnapshotsAndTracksChangedRegion) {
    const auto previous = MakeSnapshot(2, 2, std::vector<float>(16, 0.0f));
    auto currentPixels = std::vector<float>(16, 0.0f);
    currentPixels[3] = 1.0f;
    currentPixels[12] = 2.0f;
    const auto current = MakeSnapshot(2, 2, std::move(currentPixels));

    const auto difference = CompareTextureSnapshots(previous, current);

    ASSERT_TRUE(difference.has_value());
    EXPECT_EQ(difference->changedPixelCount, 2u);
    EXPECT_DOUBLE_EQ(difference->changedPixelPercentage, 50.0);
    EXPECT_EQ(difference->changedBoundsMinimum.x, 0u);
    EXPECT_EQ(difference->changedBoundsMinimum.y, 0u);
    EXPECT_EQ(difference->changedBoundsMaximum.x, 1u);
    EXPECT_EQ(difference->changedBoundsMaximum.y, 1u);
    EXPECT_DOUBLE_EQ(difference->signedDifference.channels[0].maximum, 2.0);
    EXPECT_DOUBLE_EQ(difference->absoluteDifference.channels[3].sum, 1.0);
}

TEST(RenderDebugTest, TreatsMatchingNonFiniteValuesAsUnchanged) {
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float infinity = std::numeric_limits<float>::infinity();
    const auto previous = MakeSnapshot(1, 1, {nan, infinity, 0.0f, 1.0f});
    const auto current = MakeSnapshot(1, 1, {nan, infinity, 0.0f, 1.0f});

    const auto difference = CompareTextureSnapshots(previous, current);

    ASSERT_TRUE(difference.has_value());
    EXPECT_EQ(difference->changedPixelCount, 0u);
    EXPECT_EQ(difference->signedDifference.channels[0].zeroCount, 1u);
    EXPECT_EQ(difference->signedDifference.channels[1].zeroCount, 1u);
}

TEST(RenderDebugTest, FormatsTextureDifferenceSummary) {
    const auto previous = MakeSnapshot(1, 1, {0.0f, 0.0f, 0.0f, 0.0f});
    const auto current = MakeSnapshot(1, 1, {0.0f, 0.0f, 0.0f, 1.0f});
    const auto difference = CompareTextureSnapshots(previous, current);
    ASSERT_TRUE(difference.has_value());

    const std::string output = FormatTextureDifferenceStatistics(*difference, "iteration");

    EXPECT_NE(output.find("Texture difference 'iteration': 1x1 changedPixels=1"), std::string::npos);
    EXPECT_NE(output.find("Texture 'signed delta'"), std::string::npos);
    EXPECT_NE(output.find("Texture 'absolute delta'"), std::string::npos);
}

} // namespace
