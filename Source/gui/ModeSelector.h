/*
  ==============================================================================

    ModeSelector.h
    Created: 17 Aug 2022 11:47:16pm
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "LinearSlider.h"

//==============================================================================
struct ModeSelector : juce::Component
{
    ModeSelector(juce::AudioProcessorValueTreeState& apvts);
    void paint(juce::Graphics& g) override;
    void resized() override;
private:
    std::unique_ptr<CustomLinearSlider> slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> sliderAttachment;
};
