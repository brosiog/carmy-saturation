# 🐷 Fat Bastard

A saturation/compression audio plugin inspired by Dada Life's Sausage Fattener, rebuilt from scratch with JUCE for Apple Silicon native compatibility (VST3/AU for macOS, VST3 for Linux).

## Why

Sausage Fattener by Tailored Noise/Dada Life used PPC/Intel code that relied on Rosetta 2 translation on Apple Silicon Macs. With Rosetta 2 being sunset by Apple, the original plugin has no upgrade path. Fat Bastard is a modern, Apple Silicon-native replacement with the same spirit: one knob to make things **fat**.

## DSP Chain

```
Input → Pre-Gain → Saturation (tanh waveshaper) → Compressor (envelope follower) → Tilt EQ → Output Gain → Dry/Wet Mix
```

## Controls (MVP)

| Control   | Range     | What it does                                              |
|-----------|-----------|-----------------------------------------------------------|
| FATNESS   | 0–100%    | Drives saturation + compression amount (the main event)   |
| TONE      | -100–+100 | Tilt EQ — dark (-100) ↔ bright (+100), 0 = flat           |
| OUTPUT    | -24–+24 dB| Master output level                                       |
| WET       | 0–100%    | Dry/wet mix for parallel processing                       |

## Build

### macOS (target)

```bash
cmake -B Build -G Xcode \
  -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64" \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0
cmake --build Build --config Release
```

The built VST3 will be at `Build/FatBastard_artefacts/Release/VST3/`.

### Linux (dev/test)

```bash
cmake -B Build -G "Unix Makefiles"
cmake --build Build --config Release
```

## License

MIT
