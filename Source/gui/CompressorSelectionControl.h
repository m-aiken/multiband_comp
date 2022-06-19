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
#include "TriMeter.h"
#include "BandLevel.h"

//==============================================================================
struct CompressorSelectionControl : juce::Component
{
    CompressorSelectionControl(juce::AudioProcessorValueTreeState& _apvts, const int& _bandNum);
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    void setAsSelected(bool shouldBeSelected);
    void updateMeter(const BandLevel& level);
private:
    juce::AudioProcessorValueTreeState& apvts;
    int bandNum;
    bool callbackBlocker { false };
    
    void resetSelectButtonToDefaultColors();
    void setColors(juce::Component& comp, juce::Colour fillColor);
    void setColors(juce::Component& target, const juce::Component& source);
    
    void updateButtonStates();
    void updateEnablements(juce::Button* clickedButton);
    
    juce::RangedAudioParameter* selectedBandParam; // to stop EXC_BAD_ACCESS
    juce::TextButton soloButton, muteButton, bypassButton, selectButton;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> soloAttachment,
                                                                          muteAttachment,
                                                                          bypassAttachment;
    
    std::unique_ptr<ParamListener<float>> soloParamListener,
                                          muteParamListener,
                                          bypassParamListener,
                                          selectParamListener;
    
    TriMeter meter;
};
