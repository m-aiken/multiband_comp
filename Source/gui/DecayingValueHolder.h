/*
  ==============================================================================

    DecayingValueHolder.h
    Created: 31 May 2022 12:58:00am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../Globals.h"

//==============================================================================
struct DecayingValueHolder : juce::Timer
{
    DecayingValueHolder();
    
    void updateHeldValue(const float& input);
    float getCurrentValue() const { return currentValue; }
    bool isOverThreshold() const { return currentValue > threshold; }
    void setHoldTime(const int& ms) { holdTime = ms; }
    void setDecayRate(const float& dbPerSec) { decayRatePerFrame = dbPerSec / 60; }
    
    void timerCallback() override;
private:
    float currentValue { Globals::negativeInf() };
    juce::int64 peakTime = getNow();
    float threshold = 0.f;
    juce::int64 holdTime = 1000; // 1 second
    float decayRatePerFrame { 0.f };
    float decayRateMultiplier { 1.f };
    
    static juce::int64 getNow() { return juce::Time::currentTimeMillis(); }
    void resetDecayRateMultiplier() { decayRateMultiplier = 1.f; }
};
