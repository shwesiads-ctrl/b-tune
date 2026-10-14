#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
python3 scripts/sync-web-source.py
command -v emcc >/dev/null || { echo "Install/activate the Emscripten SDK first."; exit 1; }
emcc Source/WebBridge.cpp -std=c++17 -O3 --no-entry -sSTANDALONE_WASM=1 \
  -sALLOW_MEMORY_GROWTH=0 -sINITIAL_MEMORY=33554432 -sFILESYSTEM=0 \
  -sEXPORTED_FUNCTIONS='["_mt_init","_mt_left","_mt_right","_mt_set","_mt_process","_mt_pitch","_mt_input","_mt_output","_mt_auto","_mt_progress","_mt_auto_ready","_mt_auto_value","_mt_auto_ack"]' \
  -o Web/public/matale-dsp.wasm
