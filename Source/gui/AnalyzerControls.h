/*
  ==============================================================================

    AnalyzerControls.h
    Created: 29 Jul 2022 12:10:12am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "LinearSlider.h"
#include "CustomToggleButton.h"
#include "RotaryControl.h"

//==============================================================================
struct AnalyzerControls : juce::Component
{
    AnalyzerControls(juce::AudioProcessorValueTreeState& apvts);
    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    std::unique_ptr<CustomToggleButton> onOffButton;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> onOffAttachment;
    
    std::unique_ptr<RotaryControl> decayRateSlider;
    std::unique_ptr<CustomLinearSlider> prePostEqSlider, fftOrderSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> prePostEqAttachment,
                                                                          fftOrderAttachment,
                                                                          decayRateAttachment;
};
