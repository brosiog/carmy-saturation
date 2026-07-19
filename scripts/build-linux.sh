#!/bin/bash
# Build Fat Bastard for Linux (VST3/LV2)
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"

echo "=== Generating Makefiles ==="
cmake -B Build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release

echo "=== Building ==="
cmake --build Build --config Release

echo ""
echo "✅ Build complete!"
echo "   VST3: Build/FatBastard_artefacts/Release/VST3/"
echo "   LV2:  Build/FatBastard_artefacts/Release/LV2/"
