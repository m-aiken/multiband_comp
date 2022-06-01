/*
  ==============================================================================

    RotaryControl.h
    Created: 1 Jun 2022 12:30:34am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "LookAndFeel.h"

//==============================================================================
struct RotaryControl : juce::Slider
{
    RotaryControl() { setLookAndFeel(&lnf); }
    ~RotaryControl() { setLookAndFeel(nullptr); }
    void paint(juce::Graphics& g) override;
private:
    LookAndFeel lnf;
};
