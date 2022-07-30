/*
  ==============================================================================

    CustomToggleButton.h
    Created: 30 Jul 2022 5:36:51pm
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "LookAndFeel.h"

//==============================================================================
struct CustomToggleButton : juce::ToggleButton
{
    CustomToggleButton(juce::RangedAudioParameter& rap);
    ~CustomToggleButton() { setLookAndFeel(nullptr); }
    
    void paint(juce::Graphics& g) override;
private:
    LookAndFeel lnf;
    juce::RangedAudioParameter* param;
};
