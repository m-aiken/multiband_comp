/*
  ==============================================================================

    DecayingValueHolder.cpp
    Created: 31 May 2022 12:58:00am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "DecayingValueHolder.h"

//==============================================================================
DecayingValueHolder::DecayingValueHolder()
{
    setDecayRate(3.f);
    startTimerHz(60);
}

void DecayingValueHolder::updateHeldValue(const float& input)
{
    if ( input > currentValue )
    {
        peakTime = getNow();
        currentValue = input;
        resetDecayRateMultiplier();
    }
}

void DecayingValueHolder::timerCallback()
{
    if ( getNow() - peakTime > holdTime )
    {
        currentValue -= decayRatePerFrame * decayRateMultiplier;
        currentValue = juce::jlimit<float>(Globals::getNegativeInf(), Globals::getMaxDecibels(), currentValue);
        decayRateMultiplier *= 1.04f;
        
        if ( currentValue == Globals::getNegativeInf() )
        {
            resetDecayRateMultiplier();
        }
    }
}
