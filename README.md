# 🐷 Fat Bastard

A saturation/compression audio plugin inspired by Dada Life's Sausage Fattener, rebuilt from scratch with JUCE for **Apple Silicon native** compatibility.

> **Why:** The original Sausage Fattener used PPC/Intel code that relied on Rosetta 2 on Apple Silicon Macs. With Rosetta 2 being sunset, Fat Bastard is a modern, arm64-native replacement. One knob to make things **fat**.

## DSP Chain

```
Input → Tanh Waveshaper → DC Blocker → Compressor → Tilt EQ → Output → Dry/Wet
```

## Controls

| Control   | Range       | What it does                                          |
|-----------|-------------|-------------------------------------------------------|
| FATNESS   | 0–100%      | Drives saturation + compression amount (the main event)|
| TONE      | -100–+100   | Tilt EQ — dark (-) ↔ bright (+), 0 = flat             |
| OUTPUT    | -24–+24 dB  | Master output level                                   |
| WET       | 0–100%      | Dry/wet mix for parallel processing                   |

## Build (macOS)

```bash
git clone https://github.com/brosiog/fat-bastard.git
cd fat-bastard
./scripts/build-macos.sh
```

Output: `Build/FatBastard_artefacts/Release/VST3/Fat Bastard.vst3`

See [docs/BUILD.md](docs/BUILD.md) for detailed macOS build, install, signing, and notarization instructions.

## Project Status

| Area | Status |
|------|--------|
| DSP algorithms | ✅ Tested (4/4 Python tests pass) |
| Plugin structure | ✅ Complete (Processor + Editor + DSP modules) |
| macOS build | 🔧 Needs macOS (Xcode) — code is ready |
| CI pipeline | 📄 Written but needs workflow-scoped token |
| Presets | ❌ Not yet |
| Slick UI | ❌ MVP only |

See [docs/kanban.md](docs/kanban.md) for the full project board.

## File Structure

```
Source/
├── PluginProcessor.h/.cpp   # Audio processor + DSP chain
├── PluginEditor.h/.cpp      # UI editor
├── DSP/
│   ├── SaturationStage      # Tanh waveshaper + DC blocker
│   ├── CompressorStage      # RMS envelope follower + VCA
│   └── TiltFilter           # Tilt EQ (low/high shelves)
└── UI/
    └── KnobComponent        # Reusable rotary knob
```

## License

MIT
