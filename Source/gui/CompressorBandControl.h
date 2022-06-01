/*
  ==============================================================================

    CompressorBandControl.h
    Created: 1 Jun 2022 12:00:35am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RotaryControl.h"

//==============================================================================
struct CompressorBandControl : juce::Component
{
    /*
     Attack, Release, Threshold, Ratio sliders
     Meters
     
     use same LNF from SimpleEQ for rotary sliders.
     */
    
    CompressorBandControl(juce::AudioProcessorValueTreeState& _apvts);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void initRotarySettings(juce::Slider& rotaryControl, const double& rangeStart, const double& rangeEnd, const double& defaultValue);
private:
    juce::AudioProcessorValueTreeState& apvts;
    RotaryControl attackRotary, releaseRotary, thresholdRotary, makeupGainRotary, ratioRotary;
    juce::TextButton resetButton {"RESET"};
    juce::AudioProcessorValueTreeState::SliderAttachment attackAttachment, releaseAttachment, thresholdAttachment, makeupGainAttachment, ratioAttachment;
};
