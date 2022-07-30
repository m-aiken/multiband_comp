/*
  ==============================================================================

    LinearSlider.h
    Created: 30 Jul 2022 3:12:05pm
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "LookAndFeel.h"

//==============================================================================
struct CustomLinearSlider : juce::Slider
{
    CustomLinearSlider(juce::RangedAudioParameter& rap);
    ~CustomLinearSlider() { setLookAndFeel(nullptr); }
    
    void paint(juce::Graphics& g) override;
private:
    LookAndFeel lnf;
    juce::RangedAudioParameter* param;
};
