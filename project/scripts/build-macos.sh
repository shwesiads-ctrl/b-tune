#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
arch="${1:-arm64}"
case "$arch" in arm64|x86_64|'arm64;x86_64') ;; *) echo "Use arm64, x86_64 or 'arm64;x86_64'"; exit 1;; esac
extra=()
if [[ -n "${JUCE_DIR:-}" ]]; then extra+=("-DJUCE_DIR=$JUCE_DIR"); fi
cmake -S . -B "build-mac-${arch//;/_}" -DCMAKE_BUILD_TYPE=Release "-DCMAKE_OSX_ARCHITECTURES=$arch" -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 "${extra[@]}"
cmake --build "build-mac-${arch//;/_}" --config Release --parallel 2
ctest --test-dir "build-mac-${arch//;/_}" -C Release --output-on-failure
echo "Built VST3 and AU in build-mac-${arch//;/_}/MataleTune_artefacts/Release/"
