/*
  ==============================================================================

    CustomToggleButton.h
    Created: 30 Jul 2022 5:36:51pm
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
struct CustomToggleButton : juce::ToggleButton
{
    CustomToggleButton(juce::RangedAudioParameter& rap) : param(&rap) { }
    void paint(juce::Graphics& g) override;
private:
    juce::RangedAudioParameter* param;
};
