/*
  ==============================================================================

    FifoBackgroundUpdater.h
    Created: 21 Jun 2022 12:28:47am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
template<typename Value>
struct FifoBackgroundUpdater : juce::Timer
{
    FifoBackgroundUpdater(std::function<void(Value)> f, bool useEveryValue_ = false, bool updateImmediately = false)
    : useEveryValue(useEveryValue_), funcToRun(f)
    {
        if ( updateImmediately )
        {
            updateNeeded.store(1);
        }
        
        jassert(funcToRun != null);
        
        startTimerHz(60);
    }
    
    ~FifoBackgroundUpdater() override = default;
    
    void signalUpdateNeeded(Value value)
    {
        if ( sharedValueFifo.push(value) )
        {
            updateNeeded.store(updateNeeded.load() + 1);
        }
    }
    
    void timerCallback() override
    {
        auto currentUpdate = updateNeeded.load();
        if ( lastUpdate == currentUpdate )
        {
            return;
        }
        
        lastUpdate = currentUpdate;
        Value newValues;
        if ( sharedValueFifo.getNumAvailableForReading() > 0 )
        {
            if ( useEveryValue )
            {
                while ( fifo.pull(newValues) )
                {
                    funcToRun(newValues);
                }
            }
            else
            {
                while ( fifo.pull(newValues) ) { }
                funcToRun(newValues);
            }
        }
    }
private:
    bool useEveryValue = false;
    juce::Atomic<int> updateNeeded { 0 };
    int lastUpdate = 0;
    std::function<void(Value)> funcToRun;
    Fifo<Value> sharedValueFifo;
};
