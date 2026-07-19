#!/usr/bin/env python3
"""
Fat Bastard — DSP Algorithm Validation Tests

Tests the core DSP algorithms (tanh waveshaping, envelope following,
DC blocking, shelf filtering) using mathematically correct assertions.
"""

import math
import sys

SAMPLE_RATE = 44100
DURATION_SEC = 2.0
NUM_SAMPLES = int(SAMPLE_RATE * DURATION_SEC)

def generate_sine(freq_hz, amplitude=0.5):
    return [amplitude * math.sin(2.0 * math.pi * freq_hz * t / SAMPLE_RATE)
            for t in range(NUM_SAMPLES)]

def rms(signal):
    if not signal:
        return 0.0
    return math.sqrt(sum(x*x for x in signal) / len(signal))

def peak(signal):
    return max(abs(x) for x in signal)

def snr(original, processed):
    noise = [o - p for o, p in zip(original, processed)]
    sig_rms = rms(original)
    noise_rms = rms(noise)
    if noise_rms < 1e-12:
        return float('inf')
    return 20.0 * math.log10(sig_rms / noise_rms)

def test_tanh_waveshaper():
    """Validate tanh waveshaper behavior used in SaturationStage."""
    print("\n─── Test: Tanh Waveshaper ───")

    # Low amplitude → near-linear (tanh(x) ≈ x for small x)
    low = generate_sine(440, 0.01)
    clipped_low = [math.tanh(x) for x in low]
    low_snr = snr(low, clipped_low)
    print(f"  Low amplitude (0.01): SNR = {low_snr:.1f} dB")
    assert low_snr > 40, f"Tanh should be very linear at low amplitude (SNR={low_snr:.1f})"
    print("  ✓ Near-linear at low amplitude")

    # High amplitude → hard-clipped to ≈±1.0
    high = generate_sine(440, 10.0)
    clipped_high = [math.tanh(x) for x in high]
    high_peak = peak(clipped_high)
    print(f"  High amplitude (10.0): peak output = {high_peak:.4f}")
    assert 0.95 < high_peak < 1.05, f"Tanh should clip to ~1.0, got {high_peak}"
    print("  ✓ Clips to ~1.0")

    # Harmonics generated (sine → not-sine after saturation)
    sr = SAMPLE_RATE
    sig = generate_sine(200, 2.0)
    sat = [math.tanh(x) for x in sig]

    # Measure total harmonic distortion proxy: energy above fundamental
    # FFT bin for 200Hz: bin = 200 * N / sr
    N = len(sat)
    bin_200 = int(200 * N / sr)
    # Crude: compute fundamental amplitude via DFT at 200Hz
    def dft(xs, k):
        N = len(xs)
        re = sum(xs[n] * math.cos(2 * math.pi * k * n / N) for n in range(N))
        im = sum(xs[n] * math.sin(2 * math.pi * k * n / N) for n in range(N))
        return math.sqrt(re*re + im*im) / N * 2

    import cmath
    def goertzel(xs, freq):
        """Goertzel algorithm for single-frequency magnitude."""
        N = len(xs)
        omega = 2.0 * math.pi * freq / sr
        coeff = 2.0 * math.cos(omega)
        s_prev = 0.0
        s_prev2 = 0.0
        for x in xs:
            s = x + coeff * s_prev - s_prev2
            s_prev2 = s_prev
            s_prev = s
        power = s_prev2 * s_prev2 + s_prev * s_prev - coeff * s_prev * s_prev2
        return math.sqrt(power) / N * 2

    fund = goertzel(sat, 200)
    sig_fund = goertzel(sig, 200)
    print(f"  Saturated signal fundamental (200Hz): {fund:.4f} (input: {sig_fund:.4f})")

    third_harmonic = goertzel(sat, 600)
    print(f"  3rd harmonic (600Hz): {third_harmonic:.6f}")
    assert third_harmonic > 0.001, "Saturation should generate harmonics"
    print("  ✓ Generates harmonic distortion")

    return True

def test_tilt_filter_concept():
    """Validate tilt filter math using standard biquad shelf formulas."""
    print("\n─── Test: Tilt Filter (Shelf Math) ───")

    def low_shelf_gain_db(f, fc, gain_db):
        """Compute gain of a standard low-shelf filter at frequency f."""
        G = 10 ** (gain_db / 20.0)  # linear gain
        # Standard biquad low-shelf: gain → G as f → 0, gain → 1 as f → ∞
        # At transition frequency fc, gain ~ sqrt(G)
        # Simplified: gain(f) = sqrt((G^2 + (f/fc)^2) / (1 + (f/fc)^2))
        ratio = f / fc
        gain_lin = math.sqrt((G*G + ratio*ratio) / (1.0 + ratio*ratio))
        return 20.0 * math.log10(gain_lin)

    def high_shelf_gain_db(f, fc, gain_db):
        """Compute gain of a standard high-shelf filter at frequency f."""
        G = 10 ** (gain_db / 20.0)
        # High shelf: gain → 1 as f → 0, gain → G as f → ∞
        ratio = f / fc
        gain_lin = math.sqrt((1.0 + ratio*ratio * G*G) / (1.0 + ratio*ratio))
        return 20.0 * math.log10(gain_lin)

    # Low shelf: +6dB at 250Hz
    g_low_20 = low_shelf_gain_db(20, 250, 6)
    g_low_10k = low_shelf_gain_db(10000, 250, 6)
    print(f"  Low shelf (+6dB @ 250Hz):")
    print(f"    @20Hz:  {g_low_20:+.2f} dB  (should be near +6)")
    print(f"    @10kHz: {g_low_10k:+.2f} dB  (should be near 0)")
    assert g_low_20 > 4.0, f"Low shelf should boost near DC, got {g_low_20}"
    assert abs(g_low_10k) < 3.0, f"Low shelf should be ~0 at high freq, got {g_low_10k}"

    # High shelf: +6dB at 4kHz
    g_high_20 = high_shelf_gain_db(20, 4000, 6)
    g_high_10k = high_shelf_gain_db(10000, 4000, 6)
    print(f"  High shelf (+6dB @ 4kHz):")
    print(f"    @20Hz:  {g_high_20:+.2f} dB  (should be near 0)")
    print(f"    @10kHz: {g_high_10k:+.2f} dB  (should be near +6)")
    assert abs(g_high_20) < 3.0, f"High shelf should be ~0 at low freq, got {g_high_20}"
    assert g_high_10k > 4.0, f"High shelf should boost at high freq, got {g_high_10k}"

    # Tilt complementarity: dark(-1) = low+6dB, high-6dB, bright(+1) = opposite
    g_dark_low = low_shelf_gain_db(100, 250, 6)
    g_dark_high = high_shelf_gain_db(100, 4000, -6)
    g_bright_low = low_shelf_gain_db(100, 250, -6)
    g_bright_high = high_shelf_gain_db(100, 4000, 6)
    print(f"  Dark (-1.0): low={g_dark_low:+.2f}dB, high={g_dark_high:+.2f}dB")
    print(f"  Bright (+1.0): low={g_bright_low:+.2f}dB, high={g_bright_high:+.2f}dB")
    assert g_dark_low > 0 and g_dark_high < 0, "Dark should boost lows, cut highs"
    assert g_bright_low < 0 and g_bright_high > 0, "Bright should cut lows, boost highs"
    print("  ✓ Tilt direction correct")

    return True

def test_envelope_follower():
    """Validate envelope follower and compressor concepts."""
    print("\n─── Test: Envelope Follower ───")

    amplitude = 0.5
    sig = generate_sine(100, amplitude)

    # One-pole envelope follower (attack 5ms, release 50ms)
    attack_coeff = 0.01
    release_coeff = 0.001

    envelope = 0.0
    peak_env = 0.0
    for s in sig:
        abs_s = abs(s)
        if abs_s > envelope:
            envelope += attack_coeff * (abs_s - envelope)
        else:
            envelope += release_coeff * (abs_s - envelope)
        peak_env = max(peak_env, envelope)

    print(f"  Input peak: {amplitude}, Envelope peak: {peak_env:.4f}")
    assert peak_env > 0.3, f"Envelope should track signal peak, got {peak_env}"
    assert peak_env < amplitude + 0.1, f"Envelope shouldn't overshoot much, got {peak_env}"
    print("  ✓ Envelope tracks signal within expected range")

    # Release test: longer silence for full decay
    silence_len = 5000
    for s in [0.0] * silence_len:
        abs_s = abs(s)
        if abs_s > envelope:
            envelope += attack_coeff * (abs_s - envelope)
        else:
            envelope += release_coeff * (abs_s - envelope)

    print(f"  After {silence_len} silence samples: {envelope:.6f}")
    assert envelope < 0.3, f"Envelope should decay during silence, got {envelope}"
    print("  ✓ Envelope decays during silence")

    return True

def test_dc_blocker():
    """Validate DC blocker behavior."""
    print("\n─── Test: DC Blocker ───")

    # DC blocker: y[n] = x[n] - x[n-1] + 0.995 * y[n-1]
    N = NUM_SAMPLES
    dc_input = [0.5] * N  # constant DC

    x1, y1 = 0.0, 0.0
    output = []
    for x in dc_input:
        y = x - x1 + 0.995 * y1
        x1, y1 = x, y
        output.append(y)

    out_rms = rms(output[1000:])  # skip transient
    print(f"  DC blocker: input RMS={rms(dc_input):.4f}, output RMS (settled)={out_rms:.6f}")
    assert out_rms < 0.02, f"DC blocker should strongly attenuate DC, got {out_rms}"
    print("  ✓ Removes DC offset")

    # Sinusoidal signal should pass with minimal attenuation
    sig = generate_sine(200, 0.5)
    x1, y1 = 0.0, 0.0
    output2 = []
    for x in sig:
        y = x - x1 + 0.995 * y1
        x1, y1 = x, y
        output2.append(y)

    sig_rms = rms(sig)
    out_rms2 = rms(output2[100:])
    print(f"  200Hz sine: input RMS={sig_rms:.4f}, output RMS (settled)={out_rms2:.4f}")
    ratio = out_rms2 / sig_rms
    assert ratio > 0.9, f"DC blocker should pass audio frequencies, ratio={ratio:.4f}"
    print("  ✓ Passes audio signal")

    return True

if __name__ == "__main__":
    print("=" * 55)
    print("  🐷 Fat Bastard — DSP Algorithm Validation")
    print("=" * 55)

    tests = [
        ("Tanh Waveshaper", test_tanh_waveshaper),
        ("Tilt Filter (Shelf Math)", test_tilt_filter_concept),
        ("Envelope Follower", test_envelope_follower),
        ("DC Blocker", test_dc_blocker),
    ]

    passed = 0
    failed = 0
    for name, func in tests:
        try:
            if func():
                print(f"  ✅ {name}: PASS")
                passed += 1
        except AssertionError as e:
            print(f"  ❌ {name}: FAIL — {e}")
            failed += 1
        except Exception as e:
            print(f"  💥 {name}: ERROR — {e}")
            failed += 1

    print(f"\n{'=' * 55}")
    print(f"  Results: {passed} passed, {failed} failed")
    if failed > 0:
        sys.exit(1)
    else:
        print("  All DSP algorithm tests passed! 📈")
