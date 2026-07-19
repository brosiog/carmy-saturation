#include "KnobComponent.h"

KnobComponent::KnobComponent (juce::AudioProcessorValueTreeState& apvts,
                              juce::ParameterID paramID,
                              const juce::String& labelText)
    : slider (juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow),
      attachment (apvts, paramID.getParamID(), slider)
{
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
    addAndMakeVisible (slider);

    label.setText (labelText, juce::dontSendNotification);
    label.setFont (juce::Font (12.0f, juce::Font::bold));
    label.setColour (juce::Label::textColourId, findColour (juce::Label::textColourId));
    label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (label);
}

KnobComponent::~KnobComponent()
{
}

void KnobComponent::resized()
{
    auto bounds = getLocalBounds();
    label.setBounds (bounds.removeFromTop (16));
    slider.setBounds (bounds);
}
