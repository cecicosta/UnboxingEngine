# Voxelization Lab (Web)

[Open the browser demo](https://cecicosta.github.io/UnboxingEngine/voxelization/).

Choose a torus, sphere, or cube and change the sweep resolution to voxelize it
again. Compare the input mesh and occupied cells in **Overlay**, lower the
highest visible Z layer to inspect the interior, and click a cell or the mesh
to select its scan column. Drag to orbit.

The worker runs the same C++ intersection and Bresenham fill as the desktop
sample, preserving the port's grid rounding and endpoint rules. The browser
shows the actual intersections of the selected scan ray, not an approximation
from the renderer. Rays exactly on shared triangle edges can miss or report a
single intersection, as in the native port. See the
[shared web build instructions](../WebOctree/README.md).
