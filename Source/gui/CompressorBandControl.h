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
#include "ParamListener.h"

//==============================================================================
struct CompressorBandControl : juce::Component
{
    CompressorBandControl(juce::AudioProcessorValueTreeState& _apvts);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    void resetHelper(const juce::String& paramName);
    void resetHelper(juce::RangedAudioParameter* param, const float& newValue);
    void resetParamsToDefault();
    
    void updateAttachments();
    void updateEnablements();
    int getSelectedBand();
private:
    juce::AudioProcessorValueTreeState& apvts;
    
    std::unique_ptr<RotaryControl> attackRotary, releaseRotary, thresholdRotary, makeupGainRotary, ratioRotary;
    
    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> attackAttachment,
                                releaseAttachment,
                                thresholdAttachment,
                                makeupGainAttachment,
                                ratioAttachment;
    
    juce::TextButton resetButton {"R"};
    
    std::unique_ptr<ParamListener<float>> selectedBandListener;
    
    std::vector<std::unique_ptr<ParamListener<float>>> muteListeners, soloListeners, bypassListeners;
    
    juce::TextButton bandNumButton;
};
