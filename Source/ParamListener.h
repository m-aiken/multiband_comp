/*
  ==============================================================================

    ParamListener.h
    Created: 5 Jun 2022 2:37:56pm
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
template<typename T>
struct ParamListener : juce::Timer
{
    ParamListener(juce::RangedAudioParameter& rap, std::function<void()>& valueChangedHandler)
    : param(&rap), handleValueChanged(valueChangedHandler)
    {
        jassert(param != nullptr);
        jassert(handleValueChanged != nullptr);
        
        paramValue = param->getValue();
        startTimerHz(60);
    }
    
    ~ParamListener()
    {
        stopTimer();
    }
    
    void timerCallback() override
    {
        auto newValue = param->getValue();
        if ( newValue != paramValue )
        {
            paramValue = newValue;

            handleValueChanged(param->convertFrom0to1(paramValue));
        }
    }
    
private:
    T paramValue;
    juce::RangedAudioParameter* param;
    std::function<void()> handleValueChanged;
};
