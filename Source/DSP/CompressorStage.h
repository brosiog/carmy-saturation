#pragma once

#include <juce_dsp/juce_dsp.h>

/**
    VCA-style compressor with envelope follower gain reduction.

    Implements an RMS envelope follower with soft-knee threshold at -20 dB,
    variable ratio from 1:1 to 10:1, and auto-makeup gain.
    Single-pole smoothing is used for envelope detection (attack ≈ 5 ms,
    release ≈ 50 ms).

    This is an MVP compressor — no DC blocker, no stereo linking, no sidechain
    EQ. Pure gain computer + gain reduction.
*/
class CompressorStage
{
public:
    CompressorStage();
    ~CompressorStage() = default;

    /** Initialise the compressor for the given process spec.
        Computes sample-rate-dependent coefficients and resets state. */
    void prepare (const juce::dsp::ProcessSpec& spec);

    /** Reset envelope follower state. */
    void reset();

    /** Set compression amount [0, 1].
        0.0  → no compression (unity gain, 1:1 ratio)
        1.0  → maximum compression (10:1 ratio, +3 dB makeup) */
    void setAmount (float amount);

    /** Process a block of audio, applying gain reduction. */
    void process (const juce::dsp::ProcessContextReplacing<float>& context);

private:
    /** Compute gain-reduction dB for a given RMS envelope level (in dB). */
    float calcGainReduction (float envelopeDb) const noexcept;

    // ── Time constants (seconds) ────────────────────────────────────────
    static constexpr float attackTimeSec  = 0.005f;  // 5 ms
    static constexpr float releaseTimeSec = 0.050f;  // 50 ms
    static constexpr float kneeDb         = 6.0f;    // 6 dB soft-knee width
    static constexpr float thresholdDb    = -20.0f;  // threshold level

    // ── Sample rate & coeffs (set in prepare) ───────────────────────────
    double sampleRate    = 44100.0;
    float  attackCoeff   = 0.0f;
    float  releaseCoeff  = 0.0f;
    int    rmsWindowSize = 882;  // ~20 ms @ 44.1 kHz (informational)

    // ── Envelope follower state ─────────────────────────────────────────
    float envelope = 0.0f;  // squared-signal envelope (smoothed)

    // ── Parameters ──────────────────────────────────────────────────────
    float amount     = 0.0f;
    float ratio      = 1.0f;  // 1:1 → 10:1
    float makeupGain = 1.0f;  // linear

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CompressorStage)
};
