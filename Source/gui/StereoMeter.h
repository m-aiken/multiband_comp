/*
  ==============================================================================

    StereoMeter.h
    Created: 31 May 2022 1:07:26am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../PluginProcessor.h"
#include "Meter.h"
#include "DbScale.h"

//==============================================================================
struct StereoMeter : juce::Component
{
    StereoMeter();
    void resized() override;
    void update(const MeterValues& meterValues);
private:
    Meter meterL{"L", 95.f}, meterR{"R", 95.f};
    DbScale dbScale;
};
