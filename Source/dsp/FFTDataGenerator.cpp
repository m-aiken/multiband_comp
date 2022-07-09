/*
  ==============================================================================

    FFTDataGenerator.cpp
    Created: 5 Jul 2022 12:17:09am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "FFTDataGenerator.h"
#include "../Globals.h"

//==============================================================================
void FFTDataGenerator::produceFFTDataForRendering(const juce::AudioBuffer<float>& audioData)
{
    std::fill(fftData.begin(), fftData.end(), 0.f);
    
    auto readIndex = audioData.getReadPointer(0);
    std::copy(readIndex, readIndex + getFFTSize(), fftData.begin());
    
    window->multiplyWithWindowingTable(fftData.data(), getFFTSize());
    forwardFFT->performFrequencyOnlyForwardTransform(fftData.data(), true);
    
    auto numBins = static_cast<int>(getFFTSize() * 0.5);
    juce::FloatVectorOperations::multiply(fftData.data(), 1.f / static_cast<float>(numBins), numBins+1);
    
    for ( auto i = 0; i < numBins + 1; ++i )
    {
        fftData[i] = juce::Decibels::gainToDecibels(fftData[i], Globals::getNegativeInf());
    }
    
    fftDataFifo.push(fftData);
}

void FFTDataGenerator::changeOrder(FFTOrder newOrder)
{
    order = newOrder;
    forwardFFT = std::make_unique<juce::dsp::FFT>(order);
    
    auto fftSize = getFFTSize();
    window = std::make_unique<juce::dsp::WindowingFunction<float>>(fftSize, juce::dsp::WindowingFunction<float>::blackmanHarris);
    
    std::fill(fftData.begin(), fftData.end(), 0.f);
    
    fftDataFifo.prepare(fftSize);
}
