#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class FatKnobLookAndFeel : public juce::LookAndFeel_V4
{
public:
    FatKnobLookAndFeel();
    ~FatKnobLookAndFeel() override = default;

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider& slider) override;
};

// ================================================================

class KnobComponent : public juce::Component
{
public:
    KnobComponent (juce::AudioProcessorValueTreeState& apvts,
                   juce::ParameterID paramID,
                   const juce::String& labelText);

    ~KnobComponent() override;

    void resized() override;

private:
    juce::Slider slider;
    juce::Label label;
    juce::AudioProcessorValueTreeState::SliderAttachment attachment;
    FatKnobLookAndFeel laf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KnobComponent)
};
