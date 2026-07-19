#include "PluginEditor.h"

CarmySaturationEditor::CarmySaturationEditor (CarmySaturationProcessor& p)
    : AudioProcessorEditor (&p),
      processor (p),
      driveKnob (p.apvts, { processor.driveId, 1 }, "DRIVE"),
      toneKnob    (p.apvts, { processor.toneId, 1 }, "TONE"),
      outputKnob  (p.apvts, { processor.outputId, 1 }, "OUTPUT"),
      wetKnob     (p.apvts, { processor.wetId, 1 }, "WET")
{
    // Title
    titleLabel.setText ("CARMY SATURATION", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (24.0f, juce::Font::bold));
    titleLabel.setColour (juce::Label::textColourId, juce::Colours::crimson);
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (titleLabel);

    // Knobs
    addAndMakeVisible (driveKnob);
    addAndMakeVisible (toneKnob);
    addAndMakeVisible (outputKnob);
    addAndMakeVisible (wetKnob);

    // Window size — barebones MVP
    setSize (500, 300);
}

CarmySaturationEditor::~CarmySaturationEditor()
{
}

void CarmySaturationEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF1a1a1a));

    // Subtle title underline
    g.setColour (juce::Colour (0xFF333333));
    g.drawHorizontalLine (32, 50, getWidth() - 50);
}

void CarmySaturationEditor::resized()
{
    auto area = getLocalBounds().reduced (20);

    // Title at top
    titleLabel.setBounds (area.removeFromTop (40));

    // Knob area — 4 knobs in a row
    auto knobArea = area.reduced (10, 20);
    auto knobWidth = knobArea.getWidth() / 4;

    driveKnob.setBounds (knobArea.removeFromLeft (knobWidth).reduced (6));
    toneKnob.setBounds    (knobArea.removeFromLeft (knobWidth).reduced (6));
    outputKnob.setBounds  (knobArea.removeFromLeft (knobWidth).reduced (6));
    wetKnob.setBounds     (knobArea.reduced (6));
}
