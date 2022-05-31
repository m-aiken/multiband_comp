/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PFMProject12AudioProcessorEditor::PFMProject12AudioProcessorEditor (PFMProject12AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible(inStereoMeter);
    addAndMakeVisible(outStereoMeter);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(800, 600);
    
    startTimerHz(60);
}

PFMProject12AudioProcessorEditor::~PFMProject12AudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void PFMProject12AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colour(105u, 109u, 125u)); // background
}

void PFMProject12AudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto padding = bounds.getWidth() / 40;

    auto stereoMeterWidth = padding * 4;
    
    inStereoMeter.setBounds(padding,                   // x
                            0,                         // y
                            stereoMeterWidth,          // width
                            bounds.getHeight() * 0.8); // height
    
    outStereoMeter.setBounds(bounds.getRight() - stereoMeterWidth - padding,
                             0,
                             stereoMeterWidth,
                             bounds.getHeight() * 0.8);
    
#if USE_TEST_OSC
    inStereoMeter.setBounds(padding,
                            JUCE_LIVE_CONSTANT(0),
                            stereoMeterWidth,
                            JUCE_LIVE_CONSTANT(bounds.getHeight() * 0.8));
#endif
}

void PFMProject12AudioProcessorEditor::timerCallback()
{
    handleMeterFifo(audioProcessor.inMeterValuesFifo, inMeterValues, inStereoMeter);
    handleMeterFifo(audioProcessor.outMeterValuesFifo, outMeterValues, outStereoMeter);
}

void PFMProject12AudioProcessorEditor::handleMeterFifo(Fifo<MeterValues, 20>& fifo, MeterValues& meterValues, StereoMeter& stereoMeter)
{
    if ( fifo.getNumAvailableForReading() > 0 )
    {
        while ( fifo.pull(meterValues) )
        {
            
        }
        
        stereoMeter.update(meterValues);
    }
}
