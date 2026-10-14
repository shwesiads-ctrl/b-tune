#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
cmake -S . -B build-tests -DMATALE_BUILD_PLUGIN=OFF -DCMAKE_BUILD_TYPE=Release
cmake --build build-tests --parallel 2
ctest --test-dir build-tests --output-on-failure
