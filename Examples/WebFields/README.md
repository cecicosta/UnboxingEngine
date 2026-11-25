# Fields Simulation (Web)

[Open the interactive Fields Simulation](https://cecicosta.github.io/UnboxingEngine/fields/).

The browser example uses the restored native FieldsSimulation GLSL for the
volumetric scan, potential, feedback, and display passes. The web build
extracts those shaders from the current C++ headers and adapts them for
GLSL ES 3.0. It does not replace the solver with a JavaScript approximation.

**Shader resolution** defaults to **128 × 128**, with presets from **64 × 64**
through **1024 × 1024**. It sets the size of every off-screen RGBA32F render
target, not the size of the panels. Changing it rebuilds the textures and
reseeds the fields. **Step** performs one feedback pass;
**Run** performs two passes at a time, and **Reset** restores the seeded state.

**Flow reference** defaults to **Native wiring**, where both flow inputs read
the feedback texture as in the original sample. The two inputs cancel in the
flow comparison, so the Flow panel is uniform even while the Potential panel
continues to evolve. Select **Nucleus** to compare two different fields.
No noise is added in either mode.

At **128 × 128** shader resolution, the field may take about **40,000 feedback
steps** to settle into the pattern shown in the
[stabilization recording](fields-stabilization.mp4). This is an observed
estimate, not a fixed convergence threshold.

Requires WebGL 2 with `EXT_color_buffer_float` and `EXT_float_blend`.
Run the [shared web build](../WebOctree/README.md) and open
[localhost:4173/fields/](http://127.0.0.1:4173/fields/).
