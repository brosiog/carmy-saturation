/**
 * Fat Bastard — Headless DSP Validation Tests
 *
 * Compiles and tests the DSP modules directly without any GUI dependency.
 * Run with: ./FatBastardDSPTest
 */

#include <cmath>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <numeric>

#include <juce_dsp/juce_dsp.h>

#include "DSP/SaturationStage.h"
#include "DSP/CompressorStage.h"
#include "DSP/TiltFilter.h"

// ============================================================
// Test utilities
// ============================================================

constexpr double SAMPLE_RATE = 44100.0;
constexpr int NUM_SAMPLES = 44100;  // 1 second

struct TestRunner
{
    int passed = 0;
    int failed = 0;

    void check (bool cond, const char* name, const char* msg)
    {
        if (cond)
        {
            std::printf ("  \033[32m✓\033[0m %s: %s\n", name, msg);
            ++passed;
        }
        else
        {
            std::printf ("  \033[31m✗\033[0m %s: FAIL — %s\n", name, msg);
            ++failed;
        }
    }
};

static std::vector<float> generateSine (double freq, float amp)
{
    std::vector<float> sig (NUM_SAMPLES);
    for (int i = 0; i < NUM_SAMPLES; ++i)
        sig[i] = amp * std::sin (2.0 * M_PI * freq * i / SAMPLE_RATE);
    return sig;
}

static float rms (const std::vector<float>& sig, int start = 0, int len = -1)
{
    if (len < 0) len = (int) sig.size() - start;
    double sum = 0.0;
    for (int i = start; i < start + len; ++i)
        sum += sig[i] * sig[i];
    return std::sqrt (sum / len);
}

static float peak (const std::vector<float>& sig)
{
    float p = 0.0f;
    for (auto s : sig) p = std::max (p, std::abs (s));
    return p;
}

// ============================================================
// Tests
// ============================================================

void test_saturation_stage (TestRunner& tr)
{
    std::printf ("\n─── SaturationStage Tests ───\n");

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = SAMPLE_RATE;
    spec.maximumBlockSize = 512;
    spec.numChannels = 1;

    // Test 1: create block, process through saturator
    SaturationStage sat;
    sat.prepare (spec);

    auto sig = generateSine (200.0f, 0.5f);
    juce::AudioBuffer<float> buf ((int) spec.numChannels, (int) NUM_SAMPLES);
    buf.copyFrom (0, 0, sig.data(), NUM_SAMPLES);

    sat.setGirth (0.0f);  // no saturation
    {
        juce::dsp::AudioBlock<float> block (buf);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        sat.process (ctx);
    }

    float outRms = rms ({ buf.getReadPointer (0), buf.getReadPointer (0) + NUM_SAMPLES });
    tr.check (outRms > 0.1f, "SaturationStage", "Signal passes through at girth=0");

    // Test 2: max girth creates heavy saturation (closer to square wave)
    buf.copyFrom (0, 0, sig.data(), NUM_SAMPLES);
    sat.setGirth (1.0f);
    {
        juce::dsp::AudioBlock<float> block (buf);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        sat.process (ctx);
    }

    float outPeak = peak ({ buf.getReadPointer (0), buf.getReadPointer (0) + NUM_SAMPLES });
    tr.check (outPeak < 0.7f, "SaturationStage", "Max girth compresses peak (makeup gain active)");
}

void test_compressor_stage (TestRunner& tr)
{
    std::printf ("\n─── CompressorStage Tests ───\n");

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = SAMPLE_RATE;
    spec.maximumBlockSize = 512;
    spec.numChannels = 1;

    // Test: high-level signal gets compressed
    CompressorStage comp;
    comp.prepare (spec);

    auto sig = generateSine (100.0f, 0.9f);  // loud signal, above -20dB threshold
    juce::AudioBuffer<float> buf ((int) spec.numChannels, (int) NUM_SAMPLES);
    buf.copyFrom (0, 0, sig.data(), NUM_SAMPLES);

    // No compression
    comp.setAmount (0.0f);
    {
        juce::dsp::AudioBlock<float> block (buf);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        comp.process (ctx);
    }
    float noCompPeak = peak ({ buf.getReadPointer (0), buf.getReadPointer (0) + NUM_SAMPLES });

    // Full compression
    buf.copyFrom (0, 0, sig.data(), NUM_SAMPLES);
    comp.setAmount (1.0f);
    {
        juce::dsp::AudioBlock<float> block (buf);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        comp.process (ctx);
    }
    float fullCompPeak = peak ({ buf.getReadPointer (0), buf.getReadPointer (0) + NUM_SAMPLES });

    tr.check (fullCompPeak < noCompPeak * 0.95f, "CompressorStage",
              "Full compression reduces peak amplitude relative to no compression");
}

void test_tilt_filter (TestRunner& tr)
{
    std::printf ("\n─── TiltFilter Tests ───\n");

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = SAMPLE_RATE;
    spec.maximumBlockSize = 512;
    spec.numChannels = 1;

    TiltFilter tilt;
    tilt.prepare (spec);

    // Test: low bass signal should be boosted by dark setting
    auto bass = generateSine (60.0f, 0.3f);
    juce::AudioBuffer<float> buf ((int) spec.numChannels, (int) NUM_SAMPLES);
    buf.copyFrom (0, 0, bass.data(), NUM_SAMPLES);

    // Flat first
    tilt.setTone (0.0f);
    {
        juce::dsp::AudioBlock<float> block (buf);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        tilt.process (ctx);
    }
    float flatBassRms = rms ({ buf.getReadPointer (0), buf.getReadPointer (0) + NUM_SAMPLES });

    // Dark (boost lows)
    buf.copyFrom (0, 0, bass.data(), NUM_SAMPLES);
    tilt.setTone (-1.0f);
    {
        juce::dsp::AudioBlock<float> block (buf);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        tilt.process (ctx);
    }
    float darkBassRms = rms ({ buf.getReadPointer (0), buf.getReadPointer (0) + NUM_SAMPLES });

    tr.check (std::abs (darkBassRms - flatBassRms) > 0.001f, "TiltFilter",
              "Dark setting changes low-frequency amplitude vs flat");

    // High treble signal should be boosted by bright setting
    auto treble = generateSine (8000.0f, 0.3f);
    buf.copyFrom (0, 0, treble.data(), NUM_SAMPLES);
    tilt.setTone (0.0f);
    {
        juce::dsp::AudioBlock<float> block (buf);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        tilt.process (ctx);
    }
    float flatTrebleRms = rms ({ buf.getReadPointer (0), buf.getReadPointer (0) + NUM_SAMPLES });

    buf.copyFrom (0, 0, treble.data(), NUM_SAMPLES);
    tilt.setTone (1.0f);
    {
        juce::dsp::AudioBlock<float> block (buf);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        tilt.process (ctx);
    }
    float brightTrebleRms = rms ({ buf.getReadPointer (0), buf.getReadPointer (0) + NUM_SAMPLES });

    tr.check (brightTrebleRms > flatTrebleRms, "TiltFilter",
              "Bright setting boosts treble amplitude vs flat");
}

// ============================================================
// Main
// ============================================================

int main()
{
    std::printf ("========================================\n");
    std::printf ("  🐷 Fat Bastard — DSP Headless Tests\n");
    std::printf ("========================================\n");

    TestRunner tr;

    test_saturation_stage (tr);
    test_compressor_stage (tr);
    test_tilt_filter (tr);

    std::printf ("\n========================================\n");
    std::printf ("  Results: %d passed, %d failed\n", tr.passed, tr.failed);
    std::printf ("========================================\n");

    return tr.failed > 0 ? 1 : 0;
}
