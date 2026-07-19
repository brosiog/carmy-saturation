#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"
#include "UI/KnobComponent.h"

class CarmySaturationEditor : public juce::AudioProcessorEditor
{
public:
    explicit CarmySaturationEditor (CarmySaturationProcessor&);
    ~CarmySaturationEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    CarmySaturationProcessor& processor;

    KnobComponent driveKnob;
    KnobComponent toneKnob;
    KnobComponent outputKnob;
    KnobComponent wetKnob;

    juce::Label titleLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CarmySaturationEditor)
};
