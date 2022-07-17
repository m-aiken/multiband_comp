/*
  ==============================================================================

    PathProducer.h
    Created: 17 Jul 2022 12:07:44pm
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../Globals.h"
#include "../dsp/SingleChannelSampleFifo.h"
#include "../dsp/FFTDataGenerator.h"
#include "../dsp/AnalyzerPathGenerator.h"

//==============================================================================
struct PathProducer : juce::Thread
{
    PathProducer(double _sampleRate, SingleChannelSampleFifo<juce::AudioBuffer<float>>& scsf);
    ~PathProducer() override;
    
    void run() override;
    void changeOrder(FFTOrder order);
    int getFFTSize() const;
    double getBinWidth() const;
    void pauseThread();
    void setFFTRectBounds(juce::Rectangle<float> bounds);
    
    void setDecayRate(float decayRate);
    bool pull(juce::Path& path);
    int getNumAvailableForReading() const;
    void toggleProcessing(bool toggleState);
    void changePathRange(float negativeInfinityDb, float maxDb);
private:
    SingleChannelSampleFifo<juce::AudioBuffer<float>>* singleChannelSampleFifo;
    FFTDataGenerator fftDataGenerator;
    AnalyzerPathGenerator pathGenerator;
    
    std::vector<float> renderData;
    
    void updateRenderData(std::vector<float>& renData,
                          const std::vector<float>& fftData,
                          int numBins,
                          float decayRate);
    
    juce::AudioBuffer<float> bufferForGenerator;
    
    double sampleRate;
    juce::Rectangle<float> fftBounds;
    
    std::atomic<float> decayRateInDbPerSec { 0.f },
                       negativeInfinity { Globals::getNegativeInf() },
                       maxDecibels { Globals::getMaxDecibels() };
    
    std::atomic<bool> processingIsEnabled { false };
};
