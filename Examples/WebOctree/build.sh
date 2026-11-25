#!/bin/sh
set -eu
cd "$(dirname "$0")"

mkdir -p dist/vendor
"${EMXX:-em++}" -std=c++17 -O2 -I ../../include \
    OctreeBindings.cpp ../../source/BoundingBox.cpp ../../source/UVector.cpp \
    ../../source/algorithms/LegacyCollisionAlgorithms.cpp \
    ../../source/algorithms/LegacyOctreeAlgorithms.cpp \
    -lembind -sMODULARIZE=1 -sEXPORT_ES6=1 -sENVIRONMENT=web,node \
    -sALLOW_MEMORY_GROWTH=1 -o dist/octree.mjs

cp index.html lab.mjs style.css dist/
cp ../../LICENSE.TXT dist/
cp node_modules/three/build/three.module.js node_modules/three/build/three.core.js \
    node_modules/three/examples/jsm/controls/OrbitControls.js dist/vendor/
cp node_modules/three/LICENSE dist/vendor/three-LICENSE.txt
