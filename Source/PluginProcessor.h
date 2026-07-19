#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#include "DSP/SaturationStage.h"
#include "DSP/CompressorStage.h"
#include "DSP/TiltFilter.h"

class FatBastardProcessor : public juce::AudioProcessor
{
public:
    FatBastardProcessor();
    ~FatBastardProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Fat Bastard"; }
    bool acceptsMidi() const override  { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override               { return 1; }
    int getCurrentProgram() override            { return 0; }
    void setCurrentProgram (int) override       {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    static constexpr auto fatnessId = "fatness";
    static constexpr auto toneId    = "tone";
    static constexpr auto outputId  = "output";
    static constexpr auto wetId     = "wet";

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    SaturationStage  saturation;
    CompressorStage  compressor;
    TiltFilter       tiltFilter;

    juce::dsp::Gain<float> preGain;
    juce::dsp::Gain<float> outputGain;

    std::atomic<float>* fatnessParam { nullptr };
    std::atomic<float>* toneParam    { nullptr };
    std::atomic<float>* outputParam  { nullptr };
    std::atomic<float>* wetParam     { nullptr };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FatBastardProcessor)
};
