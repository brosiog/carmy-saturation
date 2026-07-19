#pragma once

#include <juce_dsp/juce_dsp.h>

/**
    Tilt EQ filter for tone control.
    
    Implements a tilt EQ using two complementary IIR shelves:
    - Low shelf at 250 Hz, Q = 0.707
    - High shelf at 4000 Hz, Q = 0.707
    - Maximum boost/cut: ±6 dB
    
    The tilt is linear: tone = +0.5 gives low shelf at -3 dB and high shelf at +3 dB.
*/
class TiltFilter
{
public:
    /** Prepare the filter for processing.
        @param spec  The processing spec containing sample rate, block size, and channel layout.
    */
    void prepare (const juce::dsp::ProcessSpec& spec);

    /** Reset the filter state and smoothers. */
    void reset();

    /** Set the tone parameter.
        @param tone  Tone value from -1.0 (dark) to +1.0 (bright). 0.0 = flat.
    */
    void setTone (float tone);

    /** Process an audio block through the tilt EQ.
        @param context  The replacing processing context (stereo or mono).
    */
    void process (const juce::dsp::ProcessContextReplacing<float>& context);

private:
    //==============================================================================
    using Filter     = juce::dsp::IIR::Filter<float>;
    using Coeffs     = juce::dsp::IIR::Coefficients<float>;
    using Duplicator = juce::dsp::ProcessorDuplicator<Filter, Coeffs>;

    Duplicator lowShelf;
    Duplicator highShelf;

    double sampleRate { 44100.0 };

    // Smoothed tone value to prevent zippering on coefficient changes
    juce::dsp::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> smoothedTone;

    //==============================================================================
    static constexpr float lowFreq  = 250.0f;
    static constexpr float highFreq = 4000.0f;
    static constexpr float maxDb    = 6.0f;
    static constexpr float qValue   = 0.707f;

    //==============================================================================
    /** Recalculate both shelf coefficients from a single tone value.
        @param toneValue  Tone in range [-1.0, 1.0].
    */
    void updateCoefficients (float toneValue);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TiltFilter)
};
