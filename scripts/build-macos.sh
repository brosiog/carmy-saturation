#!/bin/bash
# Build Fat Bastard for macOS (Apple Silicon + Intel Universal Binary)
# Run this on a Mac with Xcode installed.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

cd "$PROJECT_DIR"

echo "=== Generating Xcode project (Universal Binary) ==="
cmake -B Build -G Xcode \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0

echo "=== Building Release ==="
cmake --build Build --config Release

echo ""
echo "✅ Build complete!"
echo "   VST3: Build/FatBastard_artefacts/Release/VST3/"
echo "   AU:   Build/FatBastard_artefacts/Release/AU/"
echo ""
echo "To install, copy to ~/Library/Audio/Plug-Ins/VST3/ or /Library/Audio/Plug-Ins/VST3/"
