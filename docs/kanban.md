# 🥫 Carmy Saturation — Project Kanban

> **Status:** Active ✅ | **Target:** macOS VST3/AU (Apple Silicon + Intel Universal Binary) + Linux VST3
> **Stack:** JUCE 8 + CMake + C++17 | **Build:** Xcode (macOS), Unix Makefiles (Linux)
> **Repo:** [github.com/brosiog/fat-bastard](https://github.com/brosiog/fat-bastard)

---

## 🔀 Active Workstreams

This project currently has **two concurrent agents** working on the adversarial review fixes. Each has its own branch to avoid conflict:

| Agent | Branch | Worktree | Status |
|-------|--------|----------|--------|
| **CarmyGPT** (me) | `fix/adversarial-carmycode` | In-repo (main checkout) | ✅ Done, unmerged |
| **Kanban agent** | `fix/adversarial-kanban` | `../carmy-saturation-kanban/` | ⏳ In progress |

### How to merge
1. Both agents finish their branches
2. Compare diffs: `git diff fix/adversarial-carmycode..fix/adversarial-kanban`
3. Pick the best version of each fix (or merge both with priority on authored quality)
4. Merge to main, delete both branches and the worktree
5. Remove worktree: `git worktree remove ../carmy-saturation-kanban`

---

## 📋 Backlog (Future)

- [ ] **DSP: Pre/Post EQ** — High-pass filter at input for DC/subsonic removal
- [ ] **DSP: Multiband option** — Split signal into low/high bands with independent saturation
- [ ] **UI: Visual meter** — Gain reduction or waveform visualization
- [ ] **UI: Slick skinning** — Full graphical skin with custom knobs, background art
- [ ] **CI: GitHub Actions** — Push workflow file with token that has workflow scope
- [ ] **Distribution: Notarization** — Apple notarization workflow for macOS distribution
- [ ] **Performance profiling** — Real-time audio thread optimization, buffer tuning
- [ ] **Preset system** — Save/load factory + user presets

## ✅ Done

### Phase 1: Foundation 🏗️
- [x] **Project scaffold** — CMakeLists.txt, directory structure, git init, .gitignore
- [x] **Kanban + docs** — Kanban board (this), README, BUILD.md (macOS build guide)

### Phase 2: Research 🔬
- [x] **Sausage Fattener DSP analysis** — Established DSP chain: tanh saturator → DC blocker → makeup gain → compression → tilt EQ
- [x] **JUCE/CMake setup** — JUCE 8 via FetchContent, Universal Binary via `CMAKE_OSX_ARCHITECTURES`
- [x] **Apple Silicon + Logic Pro X** — AU native, VST3 supported. Universal Binary for both architectures

### Phase 3: DSP Core 🔊
- [x] **SaturationStage** — Tanh waveshaper with input drive (0-36dB), DC blocker (1-pole HPF, coeff 0.995), makeup gain compensation
- [x] **CompressorStage** — RMS envelope follower (attack 5ms, release 50ms), soft-knee threshold at -20dB, ratio 1:1 to 10:1, auto-makeup +3dB
- [x] **TiltFilter** — Complementary low shelf (250Hz) + high shelf (4kHz), ±6dB, Q=0.707, zipper-free smoothing with 64-sample chunking

### Phase 4: Processor + UI 🎛️
- [x] **PluginProcessor** — APVTS with 4 float params (FATNESS, TONE, OUTPUT, WET), full DSP chain orchestration, state save/load via XML
- [x] **PluginEditor** — 4 rotary knobs with labels, dark theme (crimson accents), responsive layout
- [x] **KnobComponent** — Reusable rotary knob with custom LookAndFeel, slider attachment for APVTS binding
- [x] **build scripts** — `scripts/build-macos.sh` and `scripts/build-linux.sh`

### Phase 5: Validation ✅
- [x] **Python DSP tests** — 4/4 passing: tanh waveshaper (linearity, clipping, harmonics), DC blocker (DC removal, audio pass), envelope follower (tracking, decay), shelf filter math (boost/cut direction)
- [x] **Code review** — All DSP modules reviewed against reference implementations (Glizzyizer, ChowDSP patterns)
- [x] **Git repo** — Pushed to github.com/brosiog/fat-bastard

### Phase 6: Adversarial Review Fixes 🛠️

**Branch: `fix/adversarial-carmycode` (CarmyGPT)**

The adversarial review found 11 issues (1×P0, 3×P1, 7×P2). All were fixed on CarmyGPT's branch:

- [x] **P0: Build blocker** — `FatBastardProcessor::releaseResources()` → `CarmySaturationProcessor` (linker error from rename)
- [x] **P1: No RT allocation** — `dryBuffer` pre-allocated in `prepareToPlay()`, `copyFrom()` instead of `makeCopyOf()`
- [x] **P1: Wet crossfade** — `SmoothedValue` with 20ms ramp on wet/dry mix (eliminates clicks)
- [x] **P1: Sample-rate DC blocker** — Coefficient computed from sample rate instead of hardcoded 0.995
- [x] **P2: No resize on audio thread** — Replaced `dcBlockers.resize()` with `jassert` in `process()`
- [x] **P2: Tilt coeff cache** — Skip `updateCoefficients()` when tone hasn't changed

**Branch: `fix/adversarial-kanban` (Kanban agent)**

- [ ] **(same tasks — agent resolves independently)**

---

## Key Decisions

| Decision | Choice | Rationale |
|----------|--------|-----------|
| Plugin format | VST3 (primary), AU (secondary) | User runs Logic Pro X (supports both). VST3 is cross-platform |
| DSP framework | JUCE 8 DSP module | Industry standard, Apple Silicon native |
| Waveshaper | `std::tanh` | Simple, smooth, analog-like saturation curve |
| Build system | CMake + FetchContent | No Projucer, reproducible, CI-friendly |
| macOS arch | Universal Binary (arm64 + x86_64) | Runs natively on both Apple Silicon and Intel Macs |
| DC blocker | 1-pole HPF @ ~20Hz | Removes DC offset from asymmetric waveshaping |
| Tone control | Tilt EQ (complementary shelves) | Single knob = bass/treble balance, intuitive |
| Dry/wet mix | Post-chain, sample-accurate | Parallel processing for subtle saturation |

## DSP Chain (detailed)

```
Input (float)
  → Pre-gain (drive: 0-36dB mapped from FATNESS)
  → Tanh waveshaper (saturation)
  → DC blocker (1-pole, coeff 0.995)
  → Makeup gain (1/(1+3*girth))
  → RMS envelope follower (20ms window)
  → Soft-knee VCA gain reduction (threshold -20dB, ratio 1:1-10:1)
  → Low shelf @ 250Hz, High shelf @ 4kHz (±6dB from TONE)
  → Output gain (-24 to +24dB)
  → Dry/wet mix (0-100%)

Parameter smoothing via 1-pole LPF (saturation) and linear ramp (tilt EQ)
```

## Resources

- [Glizzyizer](https://github.com/SilasStilling/Glizzyizer) — Open-source Sausage Fattener-inspired plugin (reference implementation)
- [JUCE CMake Plugin Template](https://github.com/anthonyalfimov/JUCE-CMake-Plugin-Template) — Project scaffold
- [ChowDSP BYOD](https://github.com/Chowdhury-DSP/BYOD) — Build Your Own Distortion, DSP reference
- [JUCE DSP docs](https://docs.juce.com/master/group__juce__dsp.html) — Official JUCE DSP module documentation
