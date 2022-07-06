/*
  ==============================================================================

    FFTDataGenerator.h
    Created: 5 Jul 2022 12:17:09am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include "Fifo.h"

//==============================================================================
enum FFTOrder
{
    order2048 = 11,
    order4096 = 12,
    order8192 = 13
};

struct FFTDataGenerator
{
    void produceFFTDataForRendering(const juce::AudioBuffer<float>& audioData);
    void changeOrder(FFTOrder newOrder);
    int getFFTSize() const { return 1 << order; }
    int getNumAvailableFFTDataBlocks() const { return fftDataFifo.getNumAvailableForReading(); }
    bool getFFTData(std::vector<float>&& data) { return fftDataFifo.pull(data); }
private:
    FFTOrder order;
    std::vector<float> fftData;
    std::unique_ptr<juce::dsp::FFT> forwardFFT;
    std::unique_ptr<juce::dsp::WindowingFunction<float>> window;
    
    Fifo<std::vector<float>, 20> fftDataFifo;
};
