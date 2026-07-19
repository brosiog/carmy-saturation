#include "SaturationStage.h"

void SaturationStage::prepare (const juce::dsp::ProcessSpec& spec)
{
    dcBlockers.assign (spec.numChannels, {});
    dcCoeff = std::exp (-2.0 * M_PI * 10.0 / spec.sampleRate);
    reset();
}

void SaturationStage::reset()
{
    for (auto& b : dcBlockers)
        b.reset();
    currentDriveGain  = targetDriveGain;
    currentMakeupGain = targetMakeupGain;
}

void SaturationStage::setGirth (float girth01)
{
    girth01 = juce::jlimit (0.0f, 1.0f, girth01);

    targetDriveGain  = juce::Decibels::decibelsToGain (girth01 * maxDriveDb);
    targetMakeupGain = 1.0f / (1.0f + 3.0f * girth01);
}

void SaturationStage::process (const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& block = context.getOutputBlock();
    const auto numChannels = block.getNumChannels();
    const auto numSamples  = block.getNumSamples();

    jassert (dcBlockers.size() >= numChannels);

    constexpr float smoothing = 0.995f;

    for (size_t s = 0; s < numSamples; ++s)
    {
        currentDriveGain  = smoothing * currentDriveGain  + (1.0f - smoothing) * targetDriveGain;
        currentMakeupGain = smoothing * currentMakeupGain + (1.0f - smoothing) * targetMakeupGain;

        for (size_t ch = 0; ch < numChannels; ++ch)
        {
            float x       = block.getSample ((int) ch, (int) s);
            float driven  = x * currentDriveGain;
            float clipped = std::tanh (driven);
            float blocked = dcBlockers[ch].process (clipped, dcCoeff);
            block.setSample ((int) ch, (int) s, blocked * currentMakeupGain);
        }
    }
}
