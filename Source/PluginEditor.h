/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Globals.h"
#include "gui/StereoMeter.h"
#include "gui/CompressorBandControl.h"
#include "gui/CompressorSelectionControlContainer.h"
#include "gui/SpectrumAnalyzer.h"
#include "gui/AnalyzerControls.h"

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
    
    CompressorBandControl bandControls{audioProcessor.apvts};
    CompressorSelectionControlContainer compSelectionControls{audioProcessor.apvts};
    
    juce::ComboBox bandCountPicker;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> bandCountAttachment;
    
    size_t numActiveFilterBands { Globals::getNumMaxBands() };
    
    SpectrumAnalyzer spectrumAnalyzer;
    AnalyzerControls analyzerControls;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PFMProject12AudioProcessorEditor)
};
