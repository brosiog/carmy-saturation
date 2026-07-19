#include "CompressorStage.h"

//==============================================================================
CompressorStage::CompressorStage()
{
}

//==============================================================================
void CompressorStage::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    // Single-pole smoothing coefficients from time constants.
    // alpha = exp(-1 / (tau * fs))  — standard RC-type LPF coefficient.
    attackCoeff  = std::exp (-1.0f / static_cast<float> (attackTimeSec * sampleRate));
    releaseCoeff = std::exp (-1.0f / static_cast<float> (releaseTimeSec * sampleRate));

    // RMS window length for reference (informational — the actual envelope
    // smoothing is done by the single-pole LPF above).
    rmsWindowSize = static_cast<int> (0.02 * sampleRate);

    reset();
}

//==============================================================================
void CompressorStage::reset()
{
    envelope = 0.0f;
}

//==============================================================================
void CompressorStage::setAmount (float newAmount)
{
    amount = juce::jlimit (0.0f, 1.0f, newAmount);

    // Ratio 1:1 at amount=0 → 10:1 at amount=1
    ratio = 1.0f + amount * 9.0f;

    // Auto-makeup: 0 dB gain at amount=0 → +3 dB at amount=1
    makeupGain = std::pow (10.0f, amount * 3.0f / 20.0f);
}

//==============================================================================
void CompressorStage::process (const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& outputBlock = context.getOutputBlock();
    auto  numSamples  = outputBlock.getNumSamples();
    auto  numChannels = outputBlock.getNumChannels();

    // ── Shared envelope follower ──────────────────────────────────────────
    // We average the squared signal across all channels for each sample
    // frame, producing a single mono side-chain.  This keeps the stereo image
    // stable and matches typical VCA bus-compressor behaviour.
    //
    // Envelope topology:
    //   1. Square each channel's sample
    //   2. Average across channels (interleaved frame)
    //   3. Single-pole smoother with separate attack / release
    //   4. sqrt → RMS envelope (linear)

    for (size_t s = 0; s < numSamples; ++s)
    {
        // ── Frame power ──────────────────────────────────────────────
        float sumSq = 0.0f;
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto* data = outputBlock.getChannelPointer (ch);
            float x    = data[s];
            sumSq     += x * x;
        }
        float avgSq = sumSq / static_cast<float> (numChannels);

        // ── Envelope smoothing (attack / release) ────────────────────
        float coeff = (avgSq > envelope) ? attackCoeff : releaseCoeff;
        envelope = coeff * envelope + (1.0f - coeff) * avgSq;

        // ── RMS level ────────────────────────────────────────────────
        float envelopeLin = std::sqrt (envelope + 1.0e-10f);   // -100 dB floor

        // ── Gain computer ────────────────────────────────────────────
        float envelopeDb = 20.0f * std::log10 (envelopeLin);
        float gainReductionDb = calcGainReduction (envelopeDb);
        float totalGainDb     = -gainReductionDb;   // negative = reduction

        // Convert to linear gain and apply makeup
        float gainLin = std::pow (10.0f, totalGainDb / 20.0f) * makeupGain;

        // ── Apply gain to every channel ──────────────────────────────
        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            auto* data = outputBlock.getChannelPointer (ch);
            data[s] *= gainLin;
        }
    }
}

//==============================================================================
float CompressorStage::calcGainReduction (float envelopeDb) const noexcept
{
    // Soft-knee compressor gain computer.
    //
    //    kneeHalf = kneeDb / 2  (3 dB either side of threshold)
    //
    //  Region 1 — below knee       : GR = 0
    //  Region 2 — inside knee      : quadratic ramp from 0 → full slope
    //  Region 3 — above knee       : linear with 1 - 1/ratio slope

    constexpr float kneeHalf = kneeDb * 0.5f;   // 3 dB
    constexpr float twoKnee  = kneeDb * 2.0f;    // 12 dB — for quadratic normalisation

    if (envelopeDb <= thresholdDb - kneeHalf)
    {
        // ── Below threshold: no compression ──────────────────────────
        return 0.0f;
    }

    float slope = 1.0f - 1.0f / ratio;

    if (envelopeDb >= thresholdDb + kneeHalf)
    {
        // ── Above knee: full-ratio compression ───────────────────────
        return (envelopeDb - thresholdDb) * slope;
    }
    else
    {
        // ── Inside soft knee: quadratic transition ───────────────────
        float x = envelopeDb - (thresholdDb - kneeHalf);   // 0 → kneeDb
        return (x * x) / twoKnee * slope;
    }
}
