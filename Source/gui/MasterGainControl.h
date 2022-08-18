/*
  ==============================================================================

    MasterGainControl.h
    Created: 18 Aug 2022 11:56:42pm
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "RotaryControl.h"
#include "../Params.h"

//==============================================================================
struct MasterGainControl : juce::Component
{
//    MasterGainControl(juce::AudioProcessorValueTreeState& apvts, const juce::String& title);
    MasterGainControl(juce::AudioProcessorValueTreeState& apvts, const Params::Names title);
    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    std::unique_ptr<RotaryControl> rotary;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rotaryAttachment;
};
