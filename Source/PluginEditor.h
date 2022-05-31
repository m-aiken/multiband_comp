/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "gui/Averager.h"
#include "gui/DecayingValueHolder.h"
#include "gui/DbScale.h"
#include "gui/Meter.h"
#include "gui/StereoMeter.h"

//==============================================================================
/**
*/
class PFMProject12AudioProcessorEditor : public juce::AudioProcessorEditor, juce::Timer
{
public:
    PFMProject12AudioProcessorEditor (PFMProject12AudioProcessor&);
    ~PFMProject12AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void timerCallback() override;
    
    template<typename FifoType>
    void handleMeterFifo(FifoType& fifo, MeterValues& meterValues, StereoMeter& stereoMeter)
    {
        if ( fifo.getNumAvailableForReading() > 0 )
        {
            while ( fifo.pull(meterValues) ) { }
            stereoMeter.update(meterValues);
        }
    }

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PFMProject12AudioProcessor& audioProcessor;
    
//    juce::AudioBuffer<float> buffer;
    MeterValues inMeterValues, outMeterValues;
    
    StereoMeter inStereoMeter, outStereoMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PFMProject12AudioProcessorEditor)
};
