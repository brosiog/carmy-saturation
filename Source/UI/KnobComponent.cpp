#include "KnobComponent.h"

// ================================================================
// FatKnobLookAndFeel
// ================================================================

FatKnobLookAndFeel::FatKnobLookAndFeel()
{
    setColour (juce::Slider::rotarySliderFillColourId,     juce::Colours::crimson);
    setColour (juce::Slider::rotarySliderOutlineColourId,  juce::Colour (0xFF333333));
    setColour (juce::Slider::thumbColourId,                juce::Colours::white);
}

void FatKnobLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                            float sliderPosProportional, float rotaryStartAngle,
                                            float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (4.0f);
    auto centre = bounds.getCentre();
    auto radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;

    // Background circle
    g.setColour (findColour (juce::Slider::rotarySliderOutlineColourId));
    g.fillEllipse (bounds);

    // Arc (filled portion)
    auto arcAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);
    juce::Path arc;
    arc.addArc (bounds.getX(), bounds.getY(),
                bounds.getWidth(), bounds.getHeight(),
                rotaryStartAngle, arcAngle, true);
    auto strokeType = juce::PathStrokeType (radius * 0.15f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
    g.setColour (findColour (juce::Slider::rotarySliderFillColourId));
    g.strokePath (arc, strokeType);

    // Knob centre dot
    auto dotAngle = arcAngle - juce::MathConstants<float>::halfPi;
    auto dotRadius = radius * 0.4f;
    auto dotX = centre.x + dotRadius * std::cos (dotAngle);
    auto dotY = centre.y + dotRadius * std::sin (dotAngle);
    g.setColour (findColour (juce::Slider::thumbColourId));
    g.fillEllipse (juce::Rectangle<float> (6.0f, 6.0f).withCentre ({ dotX, dotY }));
}

// ================================================================
// KnobComponent
// ================================================================

KnobComponent::KnobComponent (juce::AudioProcessorValueTreeState& apvts,
                              juce::ParameterID paramID,
                              const juce::String& labelText)
    : slider (juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow),
      attachment (apvts, paramID.getParamID(), slider)
{
    slider.setLookAndFeel (&laf);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
    slider.setColour (juce::Slider::textBoxTextColourId, juce::Colours::white);
    slider.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xFF1a1a1a));
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0xFF333333));
    addAndMakeVisible (slider);

    label.setText (labelText, juce::dontSendNotification);
    label.setFont (juce::Font (12.0f, juce::Font::bold));
    label.setColour (juce::Label::textColourId, juce::Colours::white);
    label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (label);
}

KnobComponent::~KnobComponent()
{
    slider.setLookAndFeel (nullptr);
}

void KnobComponent::resized()
{
    auto bounds = getLocalBounds();
    label.setBounds (bounds.removeFromTop (16));
    slider.setBounds (bounds);
}
