/*
  ==============================================================================

    Params.h
    Created: 1 Jun 2022 12:48:47am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// temporarily putting these here
#define MIN_BAND_NUM 0
#define MAX_BAND_NUM 7

//==============================================================================
namespace Params
{

inline juce::String getAttackParamName(int bandNum)
{
    juce::String str;
    str << "Band " << bandNum << " Attack";
    return str;
}

inline juce::String getReleaseParamName(int bandNum)
{
    juce::String str;
    str << "Band " << bandNum << " Release";
    return str;
}

inline juce::String getThresholdParamName(int bandNum)
{
    juce::String str;
    str << "Band " << bandNum << " Threshold";
    return str;
}

inline juce::String getGainParamName(int bandNum)
{
    juce::String str;
    str << "Band " << bandNum << " Gain";
    return str;
}

inline juce::String getRatioParamName(int bandNum)
{
    juce::String str;
    str << "Band " << bandNum << " Ratio";
    return str;
}

inline juce::String getBypassParamName(int bandNum)
{
    juce::String str;
    str << "Band " << bandNum << " Bypass";
    return str;
}

inline juce::String getSoloParamName(int bandNum)
{
    juce::String str;
    str << "Band " << bandNum << " Solo";
    return str;
}

inline juce::String getMuteParamName(int bandNum)
{
    juce::String str;
    str << "Band " << bandNum << " Mute";
    return str;
}

inline juce::String getCrossoverParamName(int lowBandNum, int highBandNum)
{
    jassert(lowBandNum < highBandNum);
    jassert(lowBandNum >= MIN_BAND_NUM);
    jassert(highBandNum <= MAX_BAND_NUM);
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
