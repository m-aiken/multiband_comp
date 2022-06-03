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
    CompressorBandControl(juce::AudioProcessorValueTreeState& _apvts);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void resetHelper(const juce::String& paramName);
    void resetHelper(juce::RangedAudioParameter* param, const float& newValue);
    void resetParamsToDefault();
private:
    juce::AudioProcessorValueTreeState& apvts;
    
    std::unique_ptr<RotaryControl> attackRotary, releaseRotary, thresholdRotary, makeupGainRotary, ratioRotary;
    
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> attackAttachment,
                                releaseAttachment,
                                thresholdAttachment,
                                makeupGainAttachment,
                                ratioAttachment;
    
    Button resetButton {"R"};
};
