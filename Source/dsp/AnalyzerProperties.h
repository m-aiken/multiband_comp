/*
  ==============================================================================

    AnalyzerProperties.h
    Created: 21 Jul 2022 10:42:37pm
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "FFTOrder.h"

//==============================================================================
namespace AnalyzerProperties
{

enum ParamNames
{
    Enable_Analyzer,
    Analyzer_Decay_Rate,
    Analyzer_Points,
    Analyzer_Processing_Mode
};

enum ProcessingModes
{
    Pre,
    Post
};

const std::map<ParamNames, juce::String>& getAnalyzerParams()
{
    static std::map<ParamNames, juce::String> paramNamesMap =
    {
        { Enable_Analyzer,          "Enable Analyzer" },
        { Analyzer_Decay_Rate,      "Analyzer Decay Rate" },
        { Analyzer_Points,          "Analyzer Points" },
        { Analyzer_Processing_Mode, "Analyzer Processing Mode" }
    };
    
    return paramNamesMap;
}

const std::map<FFTOrder, juce::String>& getAnalyzerPoints()
{
    static std::map<FFTOrder, juce::String> fftOrderMap =
    {
        { order2048, "order2048" },
        { order4096, "order4096" },
        { order8192, "order8192" }
    };
    
    return fftOrderMap;
}

const std::map<ProcessingModes, juce::String>& getProcessingModes()
{
    static std::map<ProcessingModes, juce::String> processingModesMap =
    {
        { Pre,  "Pre" },
        { Post, "Post" }
    };
    
    return processingModesMap;
}

void addAnalyzerParams(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    const auto& params = getAnalyzerParams();
    const auto& fftOrders = getAnalyzerPoints();
    const auto& processingModes = getProcessingModes();
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(params.at(Analyzer_Points),
                                                            params.at(Analyzer_Points),
                                                            juce::StringArray { fftOrders.at(order2048), fftOrders.at(order4096), fftOrders.at(order8192) },
                                                            0));
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(params.at(Analyzer_Processing_Mode),
                                                            params.at(Analyzer_Processing_Mode),
                                                            juce::StringArray { processingModes.at(Pre), processingModes.at(Post) },
                                                            1));

    layout.add(std::make_unique<juce::AudioParameterBool>(params.at(Enable_Analyzer),
                                                          params.at(Enable_Analyzer),
                                                          true));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(params.at(Analyzer_Decay_Rate),
                                                           params.at(Analyzer_Decay_Rate),
                                                           juce::NormalisableRange<float>(0.f, 30.f, 0.1f, 1.f),
                                                           0.f));
}

}
