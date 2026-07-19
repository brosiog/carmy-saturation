#include "TiltFilter.h"

//==============================================================================
void TiltFilter::prepare (const juce::dsp::ProcessSpec& spec)
{
    sampleRate = spec.sampleRate;

    lowShelf.prepare (spec);
    highShelf.prepare (spec);

    // Set up smoothing: 20 ms ramp time is short enough to feel immediate
    // but long enough to eliminate audible zippering
    smoothedTone.reset (spec.sampleRate, 0.02);
    smoothedTone.setCurrentAndTargetValue (0.0f);
    lastTone = 0.0f;

    updateCoefficients (0.0f);
}

//==============================================================================
void TiltFilter::reset()
{
    lowShelf.reset();
    highShelf.reset();

    // Reset smoother to the current target without triggering a new ramp
    smoothedTone.setCurrentAndTargetValue (smoothedTone.getTargetValue());
}

//==============================================================================
void TiltFilter::setTone (float tone)
{
    tone = juce::jlimit (-1.0f, 1.0f, tone);
    smoothedTone.setTargetValue (tone);
}

//==============================================================================
void TiltFilter::updateCoefficients (float toneValue)
{
    // toneValue: -1.0 to +1.0
    //  -1.0: low shelf +6 dB, high shelf -6 dB  (dark)
    //   0.0: both 0 dB                            (flat)
    //  +1.0: low shelf -6 dB, high shelf +6 dB   (bright)
    // The tilt is perfectly linear: tone = +0.5 => low = -3 dB, high = +3 dB

    const float lowGain  = juce::Decibels::decibelsToGain (-toneValue * maxDb);
    const float highGain = juce::Decibels::decibelsToGain ( toneValue * maxDb);

    *lowShelf.state  = *Coeffs::makeLowShelf  (sampleRate, lowFreq,  qValue, lowGain);
    *highShelf.state = *Coeffs::makeHighShelf (sampleRate, highFreq, qValue, highGain);
}

//==============================================================================
void TiltFilter::process (const juce::dsp::ProcessContextReplacing<float>& context)
{
    auto& outputBlock = context.getOutputBlock();
    auto numSamples   = outputBlock.getNumSamples();

    if (smoothedTone.isSmoothing())
    {
        // Process the block in small chunks, updating coefficients each time
        // as the smoothed tone ramps. 64-sample chunks (~1.3 ms at 48 kHz)
        // keep the step between coefficient updates inaudible.
        int pos = 0;

        while (pos < numSamples)
        {
            auto currentTone = smoothedTone.getCurrentValue();
            updateCoefficients (currentTone);

            constexpr int chunkSize = 64;
            int remaining = numSamples - pos;
            int chunk = juce::jmin (chunkSize, remaining);

            // Advance the smoother by the chunk size
            for (int i = 0; i < chunk; ++i)
                smoothedTone.getNextValue();

            // Process this chunk with the current coefficients
            auto subBlock = outputBlock.getSubBlock (pos, chunk);
            juce::dsp::ProcessContextReplacing<float> subContext (subBlock);
            lowShelf.process (subContext);
            highShelf.process (subContext);

            pos += chunk;
        }
    }
    else
    {
        // Only recalc coefficients if tone has changed
        float currentTone = smoothedTone.getCurrentValue();
        if (currentTone != lastTone)
        {
            updateCoefficients (currentTone);
            lastTone = currentTone;
        }
        lowShelf.process (context);
        highShelf.process (context);
    }
}
