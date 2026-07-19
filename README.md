# 🥫 Carmy Saturation

**Apple Silicon-native saturation/compression audio plugin** — VST3, AU, LV2, Standalone.

A saturation/compression plugin inspired by the Sausage Fattener. Built with JUCE 8 + CMake, targeting macOS and Linux.

## DSP Chain

```
Input → Drive Gain → Tanh Saturator → DC Blocker → Makeup Gain
       → RMS Compressor → Tilt EQ → Output Gain → Wet/Dry Mix
```

| Stage | What it does |
|-------|-------------|
| **Drive** | 0–36 dB pre-gain mapped from the DRIVE knob |
| **Tanh Saturator** | Symmetric waveshaping via `std::tanh` — smooth, analog-like clipping |
| **DC Blocker** | 1-pole HPF (~20 Hz) removes DC offset from asymmetric waveshaping |
| **Makeup Gain** | Auto-compensates for saturation volume loss |
| **RMS Compressor** | Envelope follower (attack 5 ms, release 50 ms), soft-knee at -20 dB, ratio 1:1–10:1 |
| **Tilt EQ** | Complementary low shelf (250 Hz) + high shelf (4 kHz), ±6 dB |
| **Output Gain** | ±24 dB trim |
| **Wet/Dry Mix** | Sample-accurate crossfade with 20 ms smoothing |

## Parameters

| Knob | Range | Default | Description |
|------|-------|---------|-------------|
| DRIVE | 0–100% | 0% | Saturation amount (drive + makeup gain) |
| TONE | -100–+100% | 0% | Tilt EQ: dark ↔ bright |
| OUTPUT | -24–+24 dB | 0 dB | Output trim |
| WET | 0–100% | 100% | Dry/wet mix |

## Downloads

Grab the latest build from [Releases](https://github.com/brosiog/carmy-saturation/releases).

| Format | macOS | Linux |
|--------|-------|-------|
| VST3 | ✅ | ✅ |
| Audio Unit | ✅ | — |
| LV2 | — | ✅ |
| Standalone | ✅ | — |

## Building

### macOS (Universal Binary)

```bash
brew install cmake
git clone https://github.com/brosiog/carmy-saturation.git
cd carmy-saturation

cmake -B Build -G Xcode \
    -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=12.0
cmake --build Build --config Release
```

### Linux (VST3 + LV2)

```bash
sudo apt install cmake build-essential libasound2-dev libjack-jackd2-dev \
    libx11-dev libxext-dev libxinerama-dev libxrandr-dev \
    libxcursor-dev libfreetype-dev libfontconfig-dev

cmake -B Build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
cmake --build Build --config Release
```

## Project Structure

```
carmy-saturation/
├── CMakeLists.txt              # Build system (JUCE 8 via FetchContent)
├── Source/
│   ├── PluginProcessor.h/.cpp  # APVTS + DSP chain orchestration
│   ├── PluginEditor.h/.cpp     # Dark-theme UI with 4 rotary knobs
│   ├── DSP/
│   │   ├── SaturationStage     # Tanh waveshaper + DC blocker
│   │   ├── CompressorStage     # RMS envelope follower + VCA
│   │   └── TiltFilter          # Complementary low/high shelves
│   └── UI/
│       └── KnobComponent       # Reusable rotary knob
├── test/
│   ├── compile_check.cpp       # Compile smoke test
│   └── run_dsp_tests.cpp       # Headless DSP validation
├── scripts/
│   ├── build-macos.sh
│   └── build-linux.sh
└── .github/workflows/build.yml # CI: Linux + macOS with auval
```

## License

MIT
