/*
  ==============================================================================

    Meter.cpp
    Created: 31 May 2022 1:04:05am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "Meter.h"
#include "../ColourPalette.h"

//==============================================================================
Meter::Meter(const juce::String& label, const float& meterHeightProportion)
    : labelText(label), meterProportion(meterHeightProportion)
{
    fallingTick.setDecayRate(3.f);
}

void Meter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto componentHeight = bounds.getHeight();
    auto maxMeterHeight = componentHeight * (meterProportion / 100);
    auto meterBoundsYOffset = componentHeight - maxMeterHeight;
    
    g.fillAll(juce::Colours::black); // background
    
    auto maxMeterBounds = juce::Rectangle<int>(0,
                                               meterBoundsYOffset,
                                               bounds.getWidth(),
                                               maxMeterHeight);
    
    // Peak Meter
    g.setColour(ColourPalette::getColour(ColourPalette::MeterGreen));
    
    auto peakScaled = juce::jmap<float>(peakDb,
                                        Globals::getNegativeInf(),
                                        Globals::getMaxDecibels(),
                                        componentHeight,
                                        meterBoundsYOffset);
    
    g.fillRect(maxMeterBounds
                .withHeight(maxMeterHeight * peakScaled)
                .withY(peakScaled));
    
    // Average Meter
    g.setColour(ColourPalette::getColour(ColourPalette::MeterYellow));
    
    auto averageScaled = juce::jmap<float>(averageMeter.getAvg(),
                                           Globals::getNegativeInf(),
                                           Globals::getMaxDecibels(),
                                           componentHeight,
                                           meterBoundsYOffset);
    
    auto avgMeterWidth = bounds.getWidth() * 0.5;
    g.fillRect(maxMeterBounds
                .withHeight(maxMeterHeight * averageScaled)
                .withY(averageScaled)
                .withWidth(avgMeterWidth)
                .withX(bounds.getCentreX() - (avgMeterWidth * 0.5)));
    
    // Falling Tick
    g.setColour( fallingTick.isOverThreshold() ? ColourPalette::getColour(ColourPalette::Red) : ColourPalette::getColour(ColourPalette::Text) );
    
    auto tickValueScaled = juce::jmap<float>(fallingTick.getCurrentValue(),
                                             Globals::getNegativeInf(),
                                             Globals::getMaxDecibels(),
                                             componentHeight,
                                             meterBoundsYOffset);
            
    g.drawLine(bounds.getX(),     // startX
               tickValueScaled,   // startY
               bounds.getRight(), // endX
               tickValueScaled,   // endY
               3.f);              // line thickness
    
    
    // L/R label
    g.drawFittedText(labelText,
                     0,
                     0,
                     bounds.getWidth(),
                     componentHeight - maxMeterHeight,
                     juce::Justification::centred,
                     1);
    
    // Meter bounding box/line
    g.setColour(ColourPalette::getColour(ColourPalette::Text));
    g.drawRect(maxMeterBounds);
}

void Meter::update(const float& peakDbLevel, const float& rmsDbLevel)
{
    peakDb = peakDbLevel;
    fallingTick.updateHeldValue(peakDbLevel);
    averageMeter.add(rmsDbLevel);
    repaint();
}
