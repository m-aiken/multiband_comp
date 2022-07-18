/*
  ==============================================================================

    PathProducer.cpp
    Created: 17 Jul 2022 12:07:44pm
    Author:  Matt Aiken

  ==============================================================================
*/

#include "PathProducer.h"

//==============================================================================
PathProducer::PathProducer(double _sampleRate, SingleChannelSampleFifo<juce::AudioBuffer<float>>& scsf)
: juce::Thread("PathProducerThread"),
  singleChannelSampleFifo(&scsf),
  sampleRate(_sampleRate)
{
    startThread();
}

PathProducer::~PathProducer()
{
    notify();
    stopThread(100);
}

void PathProducer::run()
{
    while (!threadShouldExit())
    {
        if ( !processingIsEnabled.load() )
        {
            wait(10);
            continue;
        }
        
        while ( singleChannelSampleFifo->getNumCompleteBuffersAvailable() > 0 )
        {
            juce::AudioBuffer<float> tempBuffer;
            
            if ( threadShouldExit() )
                break;
            
            if ( singleChannelSampleFifo->getAudioBuffer(tempBuffer) )
            {
                auto size = juce::jmin(tempBuffer.getNumSamples(), bufferForGenerator.getNumSamples());
                auto readPtr = bufferForGenerator.getReadPointer(0, size);
                auto writePtr = bufferForGenerator.getWritePointer(0, 0);
                std::copy(readPtr, readPtr + (bufferForGenerator.getNumSamples() - size), writePtr);
                
                juce::FloatVectorOperations::copy(bufferForGenerator.getWritePointer(0, bufferForGenerator.getNumSamples() - size),
                                                  tempBuffer.getReadPointer(0, 0),
                                                  size);
                
                fftDataGenerator.produceFFTDataForRendering(bufferForGenerator);
            }
        }
        
        while ( fftDataGenerator.getNumAvailableFFTDataBlocks() > 0 )
        {
            std::vector<float> fftData;
            
            if ( threadShouldExit() )
                break;
            
            if ( fftDataGenerator.getFFTData(fftData) )
            {
                auto fftSize = getFFTSize();
                auto numBins = static_cast<int>(fftSize * 0.5);
                auto decayRate = decayRateInDbPerSec.load();
                
                updateRenderData(renderData, fftData, numBins, decayRate);
                
                pathGenerator.generatePath(renderData, fftBounds, fftSize, static_cast<float>(getBinWidth()));
            }
        }
        
        wait(10);
    }
}

void PathProducer::changeOrder(FFTOrder order)
{
    pauseThread();
    
    fftDataGenerator.changeOrder(order);
    
    auto fftSize = getFFTSize();
    renderData.clear();
    renderData.resize(static_cast<size_t>(fftSize * 2), negativeInfinity.load());
    bufferForGenerator.setSize(1, fftSize);
    
    while ( !singleChannelSampleFifo->isPrepared() )
    {
        wait(5);
    }
    
    if ( !fftBounds.isEmpty() )
    {
        startThread();
    }
}

int PathProducer::getFFTSize() const
{
    return fftDataGenerator.getFFTSize();
}

double PathProducer::getBinWidth() const
{
    return sampleRate / static_cast<double>(getFFTSize());
}

void PathProducer::pauseThread()
{
    stopThread(100);
}

void PathProducer::setFFTRectBounds(juce::Rectangle<float> bounds)
{
    pauseThread();
    
    if ( !bounds.isEmpty() )
    {
        fftBounds = bounds;
    }
    
    startThread();
}

void PathProducer::setDecayRate(float decayRate)
{
    decayRateInDbPerSec.store(decayRate);
}

bool PathProducer::pull(juce::Path& path)
{
    return pathGenerator.getPath(path);
}

int PathProducer::getNumAvailableForReading() const
{
    return pathGenerator.getNumPathsAvailable();
}

void PathProducer::toggleProcessing(bool toggleState)
{
    processingIsEnabled.store(toggleState);
}

void PathProducer::changePathRange(float negativeInfinityDb, float maxDb)
{
    negativeInfinity.store(negativeInfinityDb);
    maxDecibels.store(maxDb);
}

void PathProducer::updateRenderData(std::vector<float>& renData, const std::vector<float>& fftData, int numBins, float decayRate)
{
    if ( decayRate >= 0.f )
    {
        for ( auto i = 0; i < numBins; ++i )
        {
            auto previousValue = renData[i];
            auto candidate = fftData[i];
            auto finalValue = juce::jmax(candidate, previousValue - decayRate);
            renData[i] = juce::jlimit(negativeInfinity.load(), maxDecibels.load(), finalValue);
        }
    }
}
