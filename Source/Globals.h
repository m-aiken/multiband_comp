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

inline float getMaxDecibels() { return 18.f; }
inline float getNegativeInf() { return -96.f; }

inline int getMinBandNum() { return 0; }
inline int getMaxBandNum() { return 7; }

}
