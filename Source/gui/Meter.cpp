/*
  ==============================================================================

    Meter.cpp
    Created: 31 May 2022 1:04:05am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "Meter.h"

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
    g.setColour(juce::Colour(82u, 182u, 154u));
    
    auto peakScaled = juce::jmap<float>(peakDb,
                                        Globals::negativeInf(),
                                        Globals::maxDecibels(),
                                        componentHeight,
                                        meterBoundsYOffset);
    
    g.fillRect(maxMeterBounds
                .withHeight(maxMeterHeight * peakScaled)
                .withY(peakScaled));
    
    // Average Meter
    g.setColour(juce::Colour(217u, 237u, 146u));
    
    auto averageScaled = juce::jmap<float>(averageMeter.getAvg(),
                                           Globals::negativeInf(),
                                           Globals::maxDecibels(),
                                           componentHeight,
                                           meterBoundsYOffset);
    
    auto avgMeterWidth = bounds.getWidth() * 0.5;
    g.fillRect(maxMeterBounds
                .withHeight(maxMeterHeight * averageScaled)
                .withY(averageScaled)
                .withWidth(avgMeterWidth)
                .withX(bounds.getCentreX() - (avgMeterWidth * 0.5)));
    
    // Falling Tick
    g.setColour( fallingTick.isOverThreshold() ? juce::Colours::red : juce::Colours::white );
    
    auto tickValueScaled = juce::jmap<float>(fallingTick.getCurrentValue(),
                                             Globals::negativeInf(),
                                             Globals::maxDecibels(),
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
    g.setColour(juce::Colours::white);
    g.drawRect(maxMeterBounds);
}

void Meter::update(const float& peakDbLevel, const float& rmsDbLevel)
{
    peakDb = peakDbLevel;
    fallingTick.updateHeldValue(peakDbLevel);
    averageMeter.add(rmsDbLevel);
    repaint();
}
