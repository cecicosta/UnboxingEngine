# Radiosity Lab (Web)

[Open the browser demo](https://cecicosta.github.io/UnboxingEngine/radiosity/).

Change the left, right, or back wall's reflectance, the torus size, or the
ceiling light's intensity to recalculate the radiosity. Drag to orbit. The
default cutaway hides outward-facing walls in the viewer only; the solver
still uses the complete Cornell box.

The WebAssembly worker uses the same C++ solver as `radiosity_port` and reuses
its form factors when only reflectance or emission changes. Geometry changes
recompute them. See the [shared web build instructions](../WebOctree/README.md).
