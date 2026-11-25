#!/bin/sh
set -eu
cd "$(dirname "$0")"

mkdir -p dist/vendor dist/radiosity
"${EMXX:-em++}" -std=c++17 -O2 -I ../../include \
    OctreeBindings.cpp ../../source/BoundingBox.cpp ../../source/UVector.cpp \
    ../../source/algorithms/LegacyCollisionAlgorithms.cpp \
    ../../source/algorithms/LegacyOctreeAlgorithms.cpp \
    -lembind -sMODULARIZE=1 -sEXPORT_ES6=1 -sENVIRONMENT=web,node \
    -sALLOW_MEMORY_GROWTH=1 -o dist/octree.mjs

"${EMXX:-em++}" -std=c++17 -O2 -I ../../include -I .. \
    ../WebRadiosity/RadiosityBindings.cpp ../RadiosityCornellBox.cpp ../RadiositySolver.cpp \
    ../../source/BoundingBox.cpp ../../source/UVector.cpp \
    -lembind -sMODULARIZE=1 -sEXPORT_ES6=1 -sENVIRONMENT=worker,node \
    -sALLOW_MEMORY_GROWTH=1 -sSTACK_SIZE=262144 -o dist/radiosity/solver.mjs

cp index.html lab.mjs style.css dist/
cp ../WebRadiosity/index.html ../WebRadiosity/lab.mjs ../WebRadiosity/worker.mjs \
    ../WebRadiosity/radiosity.css dist/radiosity/
cp ../../LICENSE.TXT dist/
cp node_modules/three/build/three.module.js node_modules/three/build/three.core.js \
    node_modules/three/examples/jsm/controls/OrbitControls.js dist/vendor/
cp node_modules/three/LICENSE dist/vendor/three-LICENSE.txt
