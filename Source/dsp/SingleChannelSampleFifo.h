/*
  ==============================================================================

    SingleChannelSampleFifo.h
    Created: 3 Jul 2022 10:37:16pm
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Fifo.h"
#include "../Channel.h"

//==============================================================================
template<typename BlockType>
struct SingleChannelSampleFifo
{
    using SampleType = typename BlockType::SampleType;
    SingleChannelSampleFifo(Channel ch) : channelToUse(ch) { prepared.set(false); }
    
    void update(const BlockType& buffer)
    {
        for ( auto i = 0; i < buffer.getNumSamples(); ++i )
        {
            pushNextSampleIntoFifo(buffer.getSample(channelToUse, i));
        }
    }
    
    void pushNextSampleIntoFifo(SampleType sample)
    {
        if ( fifoIndex == getSize() )
        {
            audioBufferFifo.push(bufferToFill);
            fifoIndex = 0;
        }
        
        bufferToFill.setSample(0, fifoIndex, sample);
        ++fifoIndex;
    }
    
    void prepare(int bufferSize)
    {
        prepared.set(false);
        size.set(bufferSize);
        audioBufferFifo.prepare(bufferSize, 1);
        bufferToFill.setSize(1, bufferSize);
        fifoIndex = 0;
        prepared.set(true);
    }
    
    int getNumCompleteBuffersAvailable() const { return audioBufferFifo.getNumAvailableForReading(); }
    bool getAudioBuffer(BlockType& buf) { return audioBufferFifo.pull(buf); }
    bool isPrepared() const { return prepared.get(); }
    int getSize() const { return size.get(); }
private:
    Channel channelToUse;
    int fifoIndex = 0;
    Fifo<BlockType, 20> audioBufferFifo;
    BlockType bufferToFill;
    juce::Atomic<bool> prepared = false;
    juce::Atomic<int> size = 0;
};
