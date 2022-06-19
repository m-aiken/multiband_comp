/*
  ==============================================================================

    CompressorSelectionControlContainer.h
    Created: 13 Jun 2022 12:22:15am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../Globals.h"
#include "CompressorSelectionControl.h"
#include "BandLevel.h"

//==============================================================================
struct CompressorSelectionControlContainer : juce::Component
{
    CompressorSelectionControlContainer(juce::AudioProcessorValueTreeState& _apvts);
    void paint(juce::Graphics& g) override;
    void resized() override;
    void changeNumBandsDisplayed(int numBands);
    void updateMeters(std::array<BandLevel, Globals::getNumMaxBands()> levels);
private:
    juce::AudioProcessorValueTreeState& apvts;
    std::vector<std::unique_ptr<CompressorSelectionControl>> controls;
    int numBandsDisplayed = Globals::getNumMaxBands();
};
