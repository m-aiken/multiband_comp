/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

#define NEGATIVE_INFINITY -66.f
#define MAX_DECIBELS 12.f

//==============================================================================
template<typename T>
struct Averager
{
    Averager(size_t numElements, T initialValue)
    {
        resize(numElements, initialValue);
    }
    
    void resize(size_t numElements, T initialValue)
    {
        elements.resize(numElements);
        clear(initialValue);
    }
    
    void clear(T initialValue)
    {
        elements.assign(getSize(), initialValue);
        writeIndex.store(0);
        sum.store(static_cast<T>(std::accumulate(elements.begin(), elements.end(), 0)));
        avg.store(sum.load() / getSize());
    }
    
    size_t getSize() const
    {
        return elements.size();
    }
    
    void add(T t)
    {
        auto idx = writeIndex.load();
        auto runningTotal = sum.load();
        
        runningTotal -= elements[idx];
        runningTotal += t;
        
        elements[idx] = t;
        
        idx = (idx + 1) % getSize();
        
        sum.store(runningTotal);
        writeIndex.store(idx);
        avg.store(sum.load() / getSize());
    }
    
    T getAvg() const
    {
        return avg.load();
    }
    
private:
    std::vector<T> elements;
    std::atomic<T> avg { T() };
    std::atomic<size_t> writeIndex = 0;
    std::atomic<T> sum { 0 };
};

//==============================================================================
struct DecayingValueHolder : juce::Timer
{
    DecayingValueHolder();
    
    void updateHeldValue(const float& input);
    float getCurrentValue() const { return currentValue; }
    bool isOverThreshold() const { return currentValue > threshold; }
    void setHoldTime(const int& ms) { holdTime = ms; }
    void setDecayRate(const float& dbPerSec) { decayRatePerFrame = dbPerSec / 60; }
    
    void timerCallback() override;
private:
    float currentValue { NEGATIVE_INFINITY };
    juce::int64 peakTime = getNow();
    float threshold = 0.f;
    juce::int64 holdTime = 1000; // 1 second
    float decayRatePerFrame { 0.f };
    float decayRateMultiplier { 1.f };
    
    static juce::int64 getNow() { return juce::Time::currentTimeMillis(); }
    void resetDecayRateMultiplier() { decayRateMultiplier = 1.f; }
};

//==============================================================================
struct Tick
{
    float db { 0.f };
    int y { 0 };
};

struct DbScale : juce::Component
{
    ~DbScale() override = default;
    void paint(juce::Graphics& g) override;
    void buildBackgroundImage(int dbDivision, juce::Rectangle<int> meterBounds, int minDb, int maxDb);
    static std::vector<Tick> getTicks(int dbDivision, juce::Rectangle<int> meterBounds, int minDb, int maxDb);
private:
    juce::Image bkgd;
};

//==============================================================================
struct Meter : juce::Component
{
    Meter() { fallingTick.setDecayRate(3.f); }
    void paint(juce::Graphics& g) override;
    void update(const float& dbLevel);
private:
    float peakDb { NEGATIVE_INFINITY };
    DecayingValueHolder fallingTick;
    Averager<float> averageMeter{30, NEGATIVE_INFINITY};
};

//==============================================================================
/**
*/
class PFMProject12AudioProcessorEditor  : public juce::AudioProcessorEditor, juce::Timer
{
public:
    PFMProject12AudioProcessorEditor (PFMProject12AudioProcessor&);
    ~PFMProject12AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PFMProject12AudioProcessor& audioProcessor;
    
    juce::AudioBuffer<float> buffer;
    
    Meter meter;
    DbScale dbScale;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PFMProject12AudioProcessorEditor)
};
