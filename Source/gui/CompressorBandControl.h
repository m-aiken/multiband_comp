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
#include "LookAndFeel.h"

//==============================================================================
struct Button : juce::TextButton
{
    Button(const juce::String& buttonText);
    ~Button() { setLookAndFeel(nullptr); }
    void paint(juce::Graphics& g) override;
private:
    LookAndFeel lnf;
};

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
//    juce::TextButton resetButton {"R"};
    Button resetButton {"R"};
    juce::AudioProcessorValueTreeState::SliderAttachment attackAttachment, releaseAttachment, thresholdAttachment, makeupGainAttachment, ratioAttachment;
};
