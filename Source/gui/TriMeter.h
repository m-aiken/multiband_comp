/*
  ==============================================================================

    TriMeter.h
    Created: 18 Jun 2022 1:59:30pm
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../Globals.h"
#include "Tick.h"

//==============================================================================
struct TriMeter : juce::Component
{
    void paint(juce::Graphics& g) override;
    void resized() override;
    void update(const float& inDb, const float& outDb);
    
    std::vector<Tick> getTicks(int dbDivision);
private:
    juce::Rectangle<int> getMeterBounds();
    float inValueDb { Globals::getNegativeInf() }, outValueDb { Globals::getNegativeInf() };
    std::vector<Tick> ticks;
    Tick zeroDbTick;
};
