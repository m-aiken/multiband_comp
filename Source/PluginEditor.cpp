/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
void Meter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto componentHeight = bounds.getHeight();
    
    g.fillAll(juce::Colour(250u, 243u, 221u)); // background
    
    auto peakScaled = juce::jmap<float>(peakDb, NEGATIVE_INFINITY, MAX_DECIBELS, componentHeight, 0);
    
    g.setColour(juce::Colour(143u, 192u, 169u)); // rectangle colour
    g.fillRect(bounds.withHeight(componentHeight * peakScaled).withY(peakScaled));
}

void Meter::update(float dbLevel)
{
    peakDb = dbLevel;
    repaint();
}

//==============================================================================
PFMProject12AudioProcessorEditor::PFMProject12AudioProcessorEditor (PFMProject12AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible(meter);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(600, 400);
    
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
    auto padding = bounds.getWidth() / 20;
    meter.setBounds(padding,                             // x
                    padding,                             // y
                    padding * 2,                         // width
                    bounds.getHeight() - (padding * 2)); // height
}

void PFMProject12AudioProcessorEditor::timerCallback()
{
    if ( audioProcessor.guiFifo.getNumAvailableForReading() > 0 )
    {
        while ( audioProcessor.guiFifo.pull(buffer) )
        {
            // do nothing else - just looping through until incomingBuffer = most recent available buffer
        }
        
        auto leftChannelMag = buffer.getMagnitude(0, 0, buffer.getNumSamples());
        auto leftChannelDb = juce::Decibels::gainToDecibels(leftChannelMag, NEGATIVE_INFINITY);
        meter.update(leftChannelDb);
    }
}
