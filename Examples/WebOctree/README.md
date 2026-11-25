# Octree Ray Picking Lab (Web)

[Play the demo](https://cecicosta.github.io/UnboxingEngine/).

The browser build generates the same torus as the desktop lab using
`MeshPrimitivesUtils`, then builds and queries the engine's mesh octree in C++
compiled to WebAssembly. Three.js draws the mesh, octants, animated ray, and
first hit. Left-click or tap to fire a ray; drag to orbit. Switch to **Side** to
see the length of a ray fired from the camera rather than looking down it.
**Fire sample ray** repeats a known hit.

## Build locally

Activate Emscripten (CI uses 4.0.14), then from this directory run:

```sh
npm ci
npm run build
npm test
npm run serve
```

Open [localhost:4173](http://127.0.0.1:4173). The output is self-contained in
`dist/`; it must be served over HTTP rather than opened from disk. The build
does not require SDL, GLEW, or any Git submodules.

## GitHub Pages

Set **Settings > Pages > Build and deployment > Source** to **GitHub Actions**.
The **Publish Octree Ray Picking Lab** workflow builds and tests the module,
then publishes `dist/` on relevant pushes to `OpenGLRenderSystem` or `main`.
Allow the chosen branch in the `github-pages` environment if it has deployment
branch restrictions. Generated files are uploaded as a Pages artifact, not
committed to Git.
