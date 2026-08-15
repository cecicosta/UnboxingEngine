# Render Debugging

The render-debug API reads texture data back from the active render system and
calculates statistics on the CPU. Access it through `CCore`:

```cpp
const auto& debug = engine.GetRenderDebug();
```

Texture readback is synchronous. It can wait for pending GPU work, so use these
operations for diagnostics rather than every frame in a normal release loop.

A complete executable example is available in
`Examples/RenderDebugExample.cpp`. Build and run it with:

```bash
cmake --build build --target render_debug_example
./build/Examples/render_debug_example
```

## Texture Statistics

`IRenderDebug::InspectTexture` returns structured statistics.
`IRenderDebug::PrintTextureStatistics` reads the texture and writes the same
information to standard output.

```cpp
#include "UnboxingEngine.h"
#include "systems/IRenderDebug.h"

using namespace unboxing_engine;

systems::STextureInspectionOptions options;
options.nonZeroEpsilon = 1e-18;

engine.GetRenderDebug().PrintTextureStatistics(
    *renderToTexture.GetDstTexture(),
    "electron field",
    options);
```

The report contains separate statistics for `R`, `G`, `B`, and `A`, plus the
length of the RGB vector. Each value report contains:

- minimum and maximum finite values and their pixel coordinates
- sum and average of all finite values
- average of values classified as nonzero
- finite, zero, nonzero, positive, negative, NaN, and infinity counts
- percentage and bounds of pixels classified as nonzero

Texture coordinates use the OpenGL readback orientation. Pixel `(0, 0)` is the
lower-left pixel.

### Epsilon Semantics

`nonZeroEpsilon` affects classification, changed-pixel detection, and nonzero
bounds. A finite value is classified as zero when:

```text
abs(value) <= nonZeroEpsilon
```

It does not replace the value before calculating the minimum, maximum, sum, or
average. Consequently, a report can show a small negative minimum while showing
`negative=0` when that value is inside the configured epsilon.

Use `0.0` to count every exactly nonzero value. Use a positive epsilon to ignore
floating-point noise at a scale that is insignificant for the calculation being
debugged.

## Capturing A Snapshot

`CaptureTexture` returns both the statistics and a copy of all RGBA pixels. Keep
the snapshot when the raw values or a later comparison are needed.

```cpp
const auto snapshot = engine.GetRenderDebug().CaptureTexture(
    *renderToTexture.GetDstTexture(),
    options);

if (snapshot) {
    const auto& alpha = snapshot->statistics.channels[3];
    std::cout << "alpha sum: " << alpha.sum << '\n';
    std::cout << "alpha NaNs: " << alpha.nanCount << '\n';
}
```

`rgbaPixels` is interleaved in `R, G, B, A` order. The first component of pixel
`(x, y)` is at:

```cpp
const size_t offset =
    (static_cast<size_t>(y) * snapshot->width + x) * 4;
const float alpha = snapshot->rgbaPixels[offset + 3];
```

## Comparing Render Steps

Capture the same texture before and after a render step, then compare the two
snapshots. The comparison reports signed and absolute per-channel differences,
the number and percentage of changed pixels, and the bounding rectangle of the
changed region.

```cpp
#include <iostream>

const auto& debug = engine.GetRenderDebug();
auto previous = debug.CaptureTexture(
    *renderToTexture.GetDstTexture(), options);

engine.StepRender();

auto current = debug.CaptureTexture(
    *renderToTexture.GetDstTexture(), options);

if (previous && current) {
    const auto difference = systems::CompareTextureSnapshots(
        *previous, *current, options);
    if (difference) {
        std::cout << systems::FormatTextureDifferenceStatistics(
            *difference, "field step 0 -> 1");
    }
}
```

Snapshots can only be compared when their dimensions, format, and pixel counts
match. Matching NaNs and same-sign infinities are considered unchanged. A
transition between finite and non-finite values is considered changed.

## Diagnosing An Iterative Field

For an iterative texture calculation, compare several consecutive steps and
watch these values:

- `signed delta A sum`: net field value added or removed during one step
- `absolute delta A sum`: total activity, including changes that cancel globally
- `changedPercent`: whether propagation is local or affects the full target
- `changedBounds`: whether the active region expands gradually
- `A max` and `A min`: whether magnitude is stable or grows every step
- `nan` and `inf`: whether the numerical calculation has become invalid

For a stable local diffusion, the changed bounds normally expand from the source
region while per-pixel differences remain bounded. Rapid growth of the alpha
sum and maximum, especially throughout an already-flat region, indicates that
old state is being accumulated rather than replaced by the newly calculated
state.

When comparing growth between steps, avoid relying only on the presentation
shader. A visualization scale and output clamping can make many different field
values look equally opaque.

## CPU-Only Statistics

`CalculateTextureStatistics` can inspect an existing CPU RGBA array without a
render system or OpenGL context:

```cpp
const auto statistics = systems::CalculateTextureStatistics(
    pixels.data(),
    pixels.size(),
    width,
    height,
    systems::ETextureFormat::RGBA32F,
    options);
```

This is useful in unit tests and when validating data before uploading it to a
texture.
