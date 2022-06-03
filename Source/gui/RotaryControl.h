/*
  ==============================================================================

    RotaryControl.h
    Created: 1 Jun 2022 12:30:34am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "LookAndFeel.h"

//==============================================================================
struct RotaryControl : juce::Slider
{
    RotaryControl(juce::RangedAudioParameter& rap, const juce::String& unitSuffix, const juce::String& title);
    ~RotaryControl() { setLookAndFeel(nullptr); }
    
    struct LabelPos
    {
        float pos;
        juce::String label;
    };
    
    juce::Array<LabelPos> labels;
    
    void paint(juce::Graphics& g) override;
    juce::Rectangle<int> getRotaryBounds() const;
    int getTextHeight() const { return 13; }
private:
    LookAndFeel lnf;
    juce::RangedAudioParameter* param;
    juce::String suffix;
};
