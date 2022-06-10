/*
  ==============================================================================

    CompressorSelectionControl.h
    Created: 6 Jun 2022 11:43:32pm
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "ParamListener.h"

//==============================================================================
struct CompressorSelectionControl : juce::Component
{
    CompressorSelectionControl(juce::AudioProcessorValueTreeState& _apvts, int _bandNum);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void setAsSelected(bool shouldBeSelected);
private:
    juce::AudioProcessorValueTreeState& apvts;
    int bandNum;
    bool callbackBlocker { false };
    
    void resetSelectButtonToDefaultColors();
    void setColors(juce::Component& comp, juce::Colour fillColor, juce::Colour offColor);
    void setColors(juce::Component& comp, juce::var fillColor, juce::var offColor);
    void setColors(juce::Component& target, const juce::Component& source);
    
    void updateButtonStates();
    void updateEnablements(juce::Button* clickedButton);
        
    juce::TextButton soloButton, muteButton, bypassButton, selectButton;
    
    using Attachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<Attachment> soloAttachment,
                                muteAttachment,
                                bypassAttachment;
    
    std::unique_ptr<ParamListener<float>> soloParamListener,
                                          muteParamListener,
                                          bypassParamListener,
                                          selectParamListener;
};
