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
        { ParamNames::Enable_Analyzer,          "Enable Analyzer" },
        { ParamNames::Analyzer_Decay_Rate,      "Analyzer Decay Rate" },
        { ParamNames::Analyzer_Points,          "Analyzer Points" },
        { ParamNames::Analyzer_Processing_Mode, "Analyzer Processing Mode" }
    };
    
    return paramNamesMap;
}

const std::map<FFTOrder, juce::String>& getAnalyzerPoints()
{
    static std::map<FFTOrder, juce::String> fftOrderMap =
    {
        { FFTOrder::order2048, "order2048" },
        { FFTOrder::order4096, "order4096" },
        { FFTOrder::order8192, "order8192" }
    };
    
    return fftOrderMap;
}

const std::map<ProcessingModes, juce::String>& getProcessingModes()
{
    static std::map<ProcessingModes, juce::String> processingModesMap =
    {
        { ProcessingModes::Pre,  "Pre" },
        { ProcessingModes::Post, "Post" }
    };
    
    return processingModesMap;
}

void addAnalyzerParams(juce::AudioProcessorValueTreeState::ParameterLayout& layout)
{
    const auto& params = getAnalyzerParams();
    const auto& fftOrders = getAnalyzerPoints();
    const auto& processingModes = getProcessingModes();
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(params.at(ParamNames::Analyzer_Points),
                                                            params.at(ParamNames::Analyzer_Points),
                                                            juce::StringArray { fftOrders.at(FFTOrder::order2048), fftOrders.at(FFTOrder::order4096), fftOrders.at(FFTOrder::order8192) },
                                                            0));
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(params.at(ParamNames::Analyzer_Processing_Mode),
                                                            params.at(ParamNames::Analyzer_Processing_Mode),
                                                            juce::StringArray { processingModes.at(ProcessingModes::Pre), processingModes.at(ProcessingModes::Post) },
                                                            1));

    layout.add(std::make_unique<juce::AudioParameterBool>(params.at(ParamNames::Enable_Analyzer),
                                                          params.at(ParamNames::Enable_Analyzer),
                                                          true));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(params.at(ParamNames::Analyzer_Decay_Rate),
                                                           params.at(ParamNames::Analyzer_Decay_Rate),
                                                           juce::NormalisableRange<float>(0.f, 30.f, 0.1f, 1.f),
                                                           0.f));
}

}
