# 🐷 Fat Bastard — Project Kanban

> **Status:** Active | **Target:** macOS VST3/AU (Apple Silicon + Intel Universal Binary) + Linux VST3
> **Stack:** JUCE 8 + CMake + C++17 | **Build:** Xcode (macOS), Unix Makefiles (Linux)

---

## 📋 Backlog

- [ ] **DSP: Pre/Post EQ** — Add high-pass filter at input for DC/subsonic removal
- [ ] **DSP: Multiband option** — Split signal into low/high bands with independent saturation
- [ ] **UI: Visual meter** — Show gain reduction or waveform visualization
- [ ] **UI: Slick skinning** — Full graphical skin with custom knobs, background art
- [ ] **CI: GitHub Actions** — Auto-build macOS (VST3/AU) + Linux on push
- [ ] **Distribution: Notarization** — Apple notarization workflow for macOS distribution
- [ ] **Performance profiling** — Real-time audio thread optimization, buffer tuning
- [ ] **Preset system** — Save/load factory + user presets

## 🔨 In Progress

### Phase 1: Foundation 🏗️

- [ ] **P1.1: Project scaffold** — CMakeLists.txt, directory structure, git init
- [ ] **P1.2: Kanban + docs** — This board, README, build documentation

### Phase 2: DSP Core 🔊

- [ ] **P2.1: SaturationStage** — Tanh waveshaper with input drive, DC blocker, makeup gain
- [ ] **P2.2: CompressorStage** — Envelope follower + VCA-style gain reduction
- [ ] **P2.3: TiltFilter** — First-order shelf pair for tone control (bass↔treble)

### Phase 3: Processor + UI 🎛️

- [ ] **P3.1: PluginProcessor** — APVTS parameter layout, DSP chain orchestration, state save/load
- [ ] **P3.2: PluginEditor** — KnobComponent, FATNESS/TONE/OUTPUT/WET controls, barebones UI
- [ ] **P3.3: KnobComponent** — Reusable rotary knob with LookAndFeel customization

### Phase 4: Build & Validate ✅

- [ ] **P4.1: Linux build** — Compile VST3 on Linux, validate DSP works
- [ ] **P4.2: Test suite** — Sine sweep, null-test, parameter automation, edge cases
- [ ] **P4.3: macOS build docs** — Xcode project gen, Universal Binary, AU validation (auval), code signing

## ✅ Done

- [x] **Research: Sausage Fattener DSP** — Established DSP chain: saturator (tanh + DC blocker) → tilt EQ → additional stage → output. Reference: Glizzyizer (open-source clone), ChowDSP
- [x] **Research: JUCE/CMake setup** — JUCE 8 via FetchContent, CMake template from anthonyalfimov/JUCE-CMake-Plugin-Template. Universal Binary via `CMAKE_OSX_ARCHITECTURES="arm64;x86_64"`
- [x] **Research: Apple Silicon + Logic Pro X** — AU is native to Logic but VST3 also supported. Rosetta 2 sunset means native arm64 required. Universal Binary ensures compatibility with both Intel and Apple Silicon Macs.

---

## Key Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Plugin format | VST3 (primary), AU (secondary) | User runs Logic Pro X (supports both). VST3 is cross-platform |
| DSP framework | JUCE 8 DSP module | Industry standard, apple silicon native, well-documented |
| Waveshaper | `std::tanh` | Simple, smooth, analog-like saturation curve |
| Build system | CMake + FetchContent | No Projucer dependency, reproducible builds |
| macOS arch | Universal Binary (arm64 + x86_64) | Runs natively on both Apple Silicon and Intel Macs |
| DC blocker | 1-pole HPF @ ~20Hz | Removes DC offset from asymmetric waveshaping |

## DSP Chain (detailed)

```
Input (float) 
  → Pre-gain (drive) 
  → Tanh waveshaper (saturation) 
  → DC blocker (1-pole, coeff 0.995) 
  → Makeup gain (1/(1+3*girth)) 
  → Envelope follower (RMS, 20ms window) 
  → VCA gain reduction 
  → Tilt EQ (low shelf @ 250Hz, high shelf @ 4kHz, ±6dB) 
  → Output gain 
  → Dry/wet mix
```

## Resources

- [Glizzyizer](https://github.com/SilasStilling/Glizzyizer) — Open-source Sausage Fattener-inspired plugin (reference)
- [JUCE CMake Plugin Template](https://github.com/anthonyalfimov/JUCE-CMake-Plugin-Template) — Project scaffold reference
- [ChowDSP BYOD](https://github.com/Chowdhury-DSP/BYOD) — Build Your Own Distortion, great DSP reference
- [JUCE DSP docs](https://docs.juce.com/master/group__juce__dsp.html) — JUCE DSP module documentation
