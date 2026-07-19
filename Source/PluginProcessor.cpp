#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
    auto pctRange = juce::NormalisableRange<float> (0.0f, 100.0f, 0.01f);
    auto biRange  = juce::NormalisableRange<float> (-100.0f, 100.0f, 0.01f);
    auto dbRange  = juce::NormalisableRange<float> (-24.0f, 24.0f, 0.01f);
}

CarmySaturationProcessor::CarmySaturationProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout CarmySaturationProcessor::createLayout()
{
    using FloatParam = juce::AudioParameterFloat;

    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add (std::make_unique<FloatParam> (
        juce::ParameterID { driveId, 1 }, "Drive",   pctRange, 0.0f));

    layout.add (std::make_unique<FloatParam> (
        juce::ParameterID { toneId, 1 }, "Tone", biRange, 0.0f));

    layout.add (std::make_unique<FloatParam> (
        juce::ParameterID { outputId, 1 }, "Output", dbRange, 0.0f));

    layout.add (std::make_unique<FloatParam> (
        juce::ParameterID { wetId, 1 }, "Wet", pctRange, 100.0f));

    return layout;
}

void CarmySaturationProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels      = static_cast<juce::uint32> (getTotalNumOutputChannels());

    saturation.prepare (spec);
    compressor.prepare (spec);
    tiltFilter.prepare (spec);

    preGain.prepare (spec);
    preGain.setRampDurationSeconds (0.02);

    outputGain.prepare (spec);
    outputGain.setRampDurationSeconds (0.02);

    dryBuffer.setSize (getTotalNumOutputChannels(), samplesPerBlock);
    smoothedWet.reset (sampleRate, 0.02);
    smoothedWet.setCurrentAndTargetValue (1.0f);

    // Cache parameter pointers for realtime-safe access
    driveParam = apvts.getRawParameterValue (driveId);
    toneParam    = apvts.getRawParameterValue (toneId);
    outputParam  = apvts.getRawParameterValue (outputId);
    wetParam     = apvts.getRawParameterValue (wetId);
}

void CarmySaturationProcessor::releaseResources()
{
}

bool CarmySaturationProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& mainOut = layouts.getMainOutputChannelSet();
    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;

    return mainOut == layouts.getMainInputChannelSet();
}

void CarmySaturationProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalNumIn  = getTotalNumInputChannels();
    const auto totalNumOut = getTotalNumOutputChannels();

    for (int ch = totalNumIn; ch < totalNumOut; ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    // Read parameters (realtime-safe from atomic pointers)
    const float drive01 = driveParam->load() * 0.01f;
    const float tone11    = toneParam->load() * 0.01f;
    const float outputDb  = outputParam->load();
    const float wetPct    = wetParam->load() * 0.01f;

    // Dry buffer for wet/dry mix — pre-allocated in prepareToPlay for RT safety
    if (wetPct < 1.0f)
    {
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            dryBuffer.copyFrom (ch, 0, buffer, ch, 0, buffer.getNumSamples());
    }

    // Set DSP parameters
    saturation.setGirth (drive01);
    compressor.setAmount (drive01);
    tiltFilter.setTone (tone11);
    outputGain.setGainDecibels (outputDb);

    // Process through DSP chain
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> ctx (block);

    saturation.process (ctx);
    compressor.process (ctx);
    tiltFilter.process (ctx);
    outputGain.process (ctx);

    // Wet/dry mix with smooth crossfade
    if (wetPct < 1.0f)
    {
        smoothedWet.setTargetValue (wetPct);
        const auto numSamples = buffer.getNumSamples();
        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        {
            auto* wet = buffer.getWritePointer (ch);
            auto* dry = dryBuffer.getReadPointer (ch);
            for (int s = 0; s < numSamples; ++s)
            {
                float w = smoothedWet.getNextValue();
                wet[s] = w * wet[s] + (1.0f - w) * dry[s];
            }
        }
    }
}

juce::AudioProcessorEditor* CarmySaturationProcessor::createEditor()
{
    return new CarmySaturationEditor (*this);
}

void CarmySaturationProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
        if (auto xml = state.createXml())
            copyXmlToBinary (*xml, destData);
}

void CarmySaturationProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CarmySaturationProcessor();
}
