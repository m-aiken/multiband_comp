/*
  ==============================================================================

    SpectrumAnalyzer.h
    Created: 23 Jul 2022 12:41:32pm
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "AnalyzerBase.h"
#include "PathProducer.h"
#include "DbScale.h"
#include "ParamListener.h"
#include "../dsp/SingleChannelSampleFifo.h"
#include "../Globals.h"

//==============================================================================
struct SpectrumAnalyzer : AnalyzerBase, juce::Timer
{
    SpectrumAnalyzer(double _sampleRate,
                     SingleChannelSampleFifo<juce::AudioBuffer<float>>& leftScsf,
                     SingleChannelSampleFifo<juce::AudioBuffer<float>>& rightScsf,
                     juce::AudioProcessorValueTreeState& apvts);
    
    void timerCallback() override;
    void resized() override;
    void paint(juce::Graphics& g) override;
    void customizeScales(int lsMin, int lsMax, int rsMin, int rsMax, int division);
private:
    double sampleRate;
    juce::Path leftAnalyzerPath, rightAnalyzerPath;
    
    PathProducer leftPathProducer, rightPathProducer;
    
    bool active { false };
    
    void paintBackground(juce::Graphics& g);
    
    void setActive(bool activeState);
    void updateDecayRate(float decayRate);
    void updateOrder(float value);
    void animate();
    
    DbScale analyzerScale, eqScale;
    
    std::unique_ptr<ParamListener<float>> analyzerEnabledParamListener,
                                          analyzerDecayRateParamListener,
                                          analyzerOrderParamListener;
    
    float leftScaleMin  {Globals::getNegativeInf()},
          leftScaleMax  {Globals::getMaxDecibels()},
          rightScaleMin {Globals::getNegativeInf()},
          rightScaleMax {Globals::getMaxDecibels()};
    
    int scaleDivision { 12 };
};
