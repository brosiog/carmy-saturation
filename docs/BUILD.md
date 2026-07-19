# Building Fat Bastard for macOS

## Prerequisites

- **macOS 11.0+** (Big Sur or later) — Apple Silicon or Intel
- **Xcode 15+** — install from App Store or [developer.apple.com](https://developer.apple.com/)
- **CMake 3.22+** — `brew install cmake` or from [cmake.org](https://cmake.org/)
- **Git** — `brew install git` or Xcode Command Line Tools

## Quick Start

```bash
# Clone
git clone https://github.com/brosiog/fat-bastard.git
cd fat-bastard

# Generate Xcode project (Universal Binary)
cmake -B Build -G Xcode \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0

# Build Release
cmake --build Build --config Release
```

Or use the helper script:
```bash
./scripts/build-macos.sh
```

## Output

After building, the plugins are at:

| Format  | Path |
|---------|------|
| **VST3** | `Build/FatBastard_artefacts/Release/VST3/Fat Bastard.vst3` |
| **AU**   | `Build/FatBastard_artefacts/Release/AU/Fat Bastard.component` |
| **Standalone** | `Build/FatBastard_artefacts/Release/Standalone/Fat Bastard.app` |

## Installing in Logic Pro X

### VST3 (Recommended)
```bash
cp -r "Build/FatBastard_artefacts/Release/VST3/Fat Bastard.vst3" \
    ~/Library/Audio/Plug-Ins/VST3/
```

### Audio Unit (AU)
```bash
cp -r "Build/FatBastard_artefacts/Release/AU/Fat Bastard.component" \
    ~/Library/Audio/Plug-Ins/Components/
```

## Validating the AU with auval

Apple's `auval` tool validates Audio Unit plugins:

```bash
# List the plugin
auval -a | grep -i "fat bastard"

# Run full validation
auval -v aufx FbsT FatB

# If validation fails, check for code signing issues:
codesign -dvvv "Build/FatBastard_artefacts/Release/AU/Fat Bastard.component"
```

**Expected auval output:**
```
auval 11802ms:  validating au fatba...
FAT BASTARD 0.1.0 - Fat Bastard Audio
  - manufacturer: FbsT
  - version: 0.1.0
  - AU type: aufx (Audio Effect)
  - AU subtype: FatB
  - supported: ✓
```

## Building Only VST3 (Skip AU)

To build VST3 and Standalone only (skip AU, for faster iteration):
```bash
cmake -B Build -G Xcode \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
    -DJUCE_BUILD_AU=OFF
cmake --build Build --config Release
```

## Code Signing & Notarization (Distribution)

For distribution outside your own Mac, you need Apple Developer ID signing and notarization.

### 1. Sign the Plugin

```bash
codesign --force --sign "Developer ID Application: Your Name" \
    --deep --timestamp \
    "Build/FatBastard_artefacts/Release/VST3/Fat Bastard.vst3"

codesign --force --sign "Developer ID Application: Your Name" \
    --deep --timestamp \
    "Build/FatBastard_artefacts/Release/AU/Fat Bastard.component"
```

### 2. Package for Notarization

```bash
ditto -c -k --keepParent \
    "Build/FatBastard_artefacts/Release/VST3/Fat Bastard.vst3" \
    /tmp/FatBastard-vst3.zip
```

### 3. Submit for Notarization

```bash
xcrun notarytool submit /tmp/FatBastard-vst3.zip \
    --apple-id "your@email.com" \
    --team-id "YOUR_TEAM_ID" \
    --password "@keychain:AC_PASSWORD"
```

### 4. Staple the Ticket

```bash
xcrun stapler staple \
    "Build/FatBastard_artefacts/Release/VST3/Fat Bastard.vst3"
```

## Building for Linux (DSP Development)

If you want to test DSP changes on Linux before building on macOS:

```bash
# On a Debian/Ubuntu system with dev headers:
sudo apt install libasound2-dev libjack-jackd2-dev \
    libx11-dev libxext-dev libxinerama-dev libxrandr-dev \
    libxcursor-dev libfreetype-dev libfontconfig-dev \
    libpng-dev libjpeg-dev

cmake -B Build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build Build --config Release
```

## Troubleshooting

### "Xcode not configured" errors
```bash
sudo xcode-select --switch /Applications/Xcode.app/Contents/Developer
```

### AU validation fails
- Ensure the plugin is code-signed (even ad-hoc):  
  `codesign --force --deep --sign - "Fat Bastard.component"`
- Check Console.app for crash logs from `aureport`
- Try: `auval -v aufx FbsT FatB -w` (wait for debugger, though this is for development)

### "Plugin not found" in Logic Pro X
- Try rescan: Logic Pro X → Settings (⌘,) → Plug-in Manager → Rescan Selection
- Check the plugin is in the correct folder
- Verify architecture: `lipo -info "Fat Bastard.component/Contents/MacOS/Fat Bastard"` should show `x86_64 arm64`

### Rosetta 2 sunset warning
The Universal Binary means Intel Mac users can run the x86_64 slice. On Apple Silicon, the arm64 slice runs natively — no Rosetta 2 needed. This is the whole point of the project.

## Project Structure

```
fat-bastard/
├── CMakeLists.txt              # Build system
├── README.md                   # This file
├── docs/kanban.md              # Project kanban board
├── Source/
│   ├── CMakeLists.txt          # Source file list
│   ├── PluginProcessor.h/.cpp  # Audio processor + DSP chain
│   ├── PluginEditor.h/.cpp     # UI editor
│   ├── DSP/
│   │   ├── SaturationStage     # Tanh waveshaper + DC blocker
│   │   ├── CompressorStage     # RMS envelope follower + VCA
│   │   └── TiltFilter          # Tilt EQ (low/high shelves)
│   └── UI/
│       └── KnobComponent       # Reusable rotary knob
└── scripts/
    ├── build-macos.sh           # macOS build shortcut
    ├── build-linux.sh           # Linux build shortcut
    └── test_dsp.py              # Python DSP algorithm tests
```
