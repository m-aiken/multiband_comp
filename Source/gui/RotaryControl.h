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
    RotaryControl(juce::RangedAudioParameter& rap, const juce::String& unitSuffix);
    ~RotaryControl() { setLookAndFeel(nullptr); }
    void paint(juce::Graphics& g) override;
private:
    LookAndFeel lnf;
    juce::RangedAudioParameter* param;
    juce::String suffix;
};
