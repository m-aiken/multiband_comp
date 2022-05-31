/*
  ==============================================================================

    Meter.h
    Created: 31 May 2022 1:04:05am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../Globals.h"
#include "DecayingValueHolder.h"
#include "Averager.h"

//==============================================================================
struct Meter : juce::Component
{
    Meter(const juce::String& label, const float& meterHeightProportion);
    void paint(juce::Graphics& g) override;
    void update(const float& peakDbLevel, const float& rmsDbLevel);
    float getMeterProportion() { return meterProportion; }
private:
    float peakDb { Globals::negativeInf() };
    DecayingValueHolder fallingTick;
    Averager<float> averageMeter{30, Globals::negativeInf()};
    juce::String labelText;
    float meterProportion;
};
