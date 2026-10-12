#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target Traka -j 4
codesign --force --deep --sign - build/Traka_artefacts/Debug/Traka.app
