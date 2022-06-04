/*
  ==============================================================================

    Params.h
    Created: 1 Jun 2022 12:48:47am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Globals.h"

//==============================================================================
namespace Params
{

enum class BandControl
{
    Attack,
    Release,
    Threshold,
    Gain,
    Ratio,
    Bypass,
    Solo,
    Mute
};

static std::map<BandControl, juce::String> bandControlMap =
{
    { BandControl::Attack,    "Attack" },
    { BandControl::Release,   "Release" },
    { BandControl::Threshold, "Threshold" },
    { BandControl::Gain,      "Gain" },
    { BandControl::Ratio,     "Ratio" },
    { BandControl::Bypass,    "Bypass" },
    { BandControl::Solo,      "Solo" },
    { BandControl::Mute,      "Mute" }
};

inline juce::String getBandControlParamName(BandControl bandControl, const int& bandNum)
{
    juce::String str;
    str << "Band " << bandNum << " " << bandControlMap.at(bandControl);
    return str;
}

inline juce::String getCrossoverParamName(int lowBandNum, int highBandNum)
{
    jassert(lowBandNum < highBandNum);
    jassert(lowBandNum >= Globals::getMinBandNum());
    jassert(highBandNum <= Globals::getMaxBandNum());
    jassert(highBandNum - lowBandNum == 1);
    
    juce::String str;
    str << "Band " << lowBandNum << "-" << highBandNum << " X-Over";
    return str;
}

enum class Names
{
    Processing_Mode,
    Gain_In,
    Gain_Out
};

inline const std::map<Names, juce::String>& getParams()
{
    static std::map<Names, juce::String> params =
    {
        { Names::Processing_Mode, "Processing Mode" },
        { Names::Gain_In, "Gain In" },
        { Names::Gain_Out, "Gain Out" }
    };
    
    return params;
}

enum class ProcessingMode
{
    Stereo,
    Left,
    Right,
    Mid,
    Side
};

inline const std::map<ProcessingMode, juce::String>& getProcessingModes()
{
    static std::map<ProcessingMode, juce::String> modes =
    {
        { ProcessingMode::Stereo, "Stereo" },
        { ProcessingMode::Left,   "Left" },
        { ProcessingMode::Right,  "Right" },
        { ProcessingMode::Mid,    "Mid" },
        { ProcessingMode::Side,   "Side" }
    };
    
    return modes;
}

}
