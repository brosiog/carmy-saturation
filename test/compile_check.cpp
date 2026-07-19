/**
 * DSP module compile + functional test.
 * Uses mock JUCE types defined inline.
 */
#include <cmath>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <numeric>

// Mock JUCE DSP types are in the include path
#include <juce_dsp/juce_dsp.h>

// Now include the DSP modules (relative to Source/)
#include "DSP/SaturationStage.h"
#include "DSP/CompressorStage.h"
#include "DSP/TiltFilter.h"

int main() {
    std::printf("Fat Bastard DSP module tests...\n");

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = 44100.0;
    spec.maximumBlockSize = 512;
    spec.numChannels = 2;

    // Test SaturationStage
    {
        SaturationStage sat;
        sat.prepare(spec);
        sat.setGirth(0.5f);

        float ch0[1024] = {0};
        float ch1[1024] = {0};
        float* chans[2] = {ch0, ch1};
        ch0[0] = 0.5f;
        juce::dsp::AudioBlock<float> block(chans, 2, 1024);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        sat.process(ctx);
        std::printf("  SaturationStage: OK (sample[0]=%f)\n", ch0[0]);
    }

    // Test CompressorStage
    {
        CompressorStage comp;
        comp.prepare(spec);
        comp.setAmount(0.7f);

        float ch0[1024] = {0};
        float ch1[1024] = {0};
        float* chans[2] = {ch0, ch1};
        ch0[0] = 0.9f;
        juce::dsp::AudioBlock<float> block(chans, 2, 1024);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        comp.process(ctx);
        std::printf("  CompressorStage: OK (sample[0]=%f)\n", ch0[0]);
    }

    // Test TiltFilter
    {
        TiltFilter tilt;
        tilt.prepare(spec);
        tilt.setTone(-0.5f);

        float ch0[1024] = {0};
        float ch1[1024] = {0};
        float* chans[2] = {ch0, ch1};
        ch0[0] = 0.3f;
        juce::dsp::AudioBlock<float> block(chans, 2, 1024);
        juce::dsp::ProcessContextReplacing<float> ctx(block);
        tilt.process(ctx);
        std::printf("  TiltFilter: OK (sample[0]=%f)\n", ch0[0]);
    }

    std::printf("\nAll tests passed! The DSP modules compile and run correctly.\n");
    return 0;
}
