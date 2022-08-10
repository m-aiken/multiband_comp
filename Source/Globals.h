/*
  ==============================================================================

    Globals.h
    Created: 31 May 2022 1:11:47am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
namespace Globals
{

constexpr float getMaxDecibels() { return 24.f; }
constexpr float getNegativeInf() { return -96.f; }

constexpr int getMinBandNum() { return 0; }
constexpr int getMaxBandNum() { return 7; }

constexpr int getNumMaxBands() { return 8; }

constexpr float getMinFrequency() { return 20.f; }
constexpr float getMaxFrequency() { return 20000.f; }

constexpr float getBorderCornerRadius() { return 5.f; }
constexpr float getBorderThickness() { return 2.f; }

}
