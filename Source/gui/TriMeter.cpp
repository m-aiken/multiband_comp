/*
  ==============================================================================

    TriMeter.cpp
    Created: 18 Jun 2022 1:59:30pm
    Author:  Matt Aiken

  ==============================================================================
*/

#include "TriMeter.h"
#include "../ColourPalette.h"

//==============================================================================
void TriMeter::paint(juce::Graphics& g)
{
    auto meterBounds = getMeterBounds();
    auto meterWidth = meterBounds.getWidth() / 3;
    auto meterHeight = meterBounds.getHeight();
    
    // meters
    // in meter
    g.setColour(juce::Colours::skyblue);
    auto inDbScaled = std::floor(juce::jmap<float>(inValueDb, Globals::getNegativeInf(), Globals::getMaxDecibels(), meterHeight, 0));
    juce::Rectangle<int> inMeter(0, inDbScaled, meterWidth, meterHeight * inDbScaled);
    g.fillRect(inMeter);
    
    // out meter
    g.setColour(ColourPalette::getColour(ColourPalette::MeterGreen));
    auto outDbScaled = std::floor(juce::jmap<float>(outValueDb, Globals::getNegativeInf(), Globals::getMaxDecibels(), meterHeight, 0));
    juce::Rectangle<int> outMeter(meterWidth, outDbScaled, meterWidth, meterHeight * outDbScaled);
    g.fillRect(outMeter);

    // gain reduction
    auto gainReduction = std::floor(juce::jmap<float>(outValueDb - inValueDb, Globals::getNegativeInf(), Globals::getMaxDecibels(), meterHeight, 0));
    juce::Rectangle<int> grMeter(meterWidth * 2, gainReduction, meterWidth, meterHeight * gainReduction);
    g.fillRect(grMeter);
    
    // ticks
    g.setColour(ColourPalette::getColour(ColourPalette::Text));
    
    for ( auto i = 0; i < 3; ++i )
    {
        for ( auto j = 0; j < ticks.size(); ++j )
        {
            auto indent = j % 2 == 0 ? 2 : 10;
            g.drawLine((meterWidth * i) + indent,     // start x
                       ticks[j].y,                    // start y
                       (meterWidth * (i+1)) - indent, // end x
                       ticks[j].y,                    // end y
                       1.f);                          // line thickness
        }
    }
    
    // separation lines
    g.drawLine(0, 0, meterBounds.getRight(), 0, 1.f); // top
    g.drawLine(0, meterBounds.getBottom(), meterBounds.getRight(), meterBounds.getBottom(), 1.f); // bottom
    for ( auto i = 1; i < 3; ++i )
    {
        g.drawLine(meterWidth * i, 0, meterWidth * i, meterBounds.getBottom(), 1.f);
    }
    
    // labels
    std::array<juce::String, 3> labels = { "I", "O", "GR" };
    for ( auto label = 0; label < labels.size(); ++label )
    {
        g.drawFittedText(labels[label],                // text
                         meterWidth * label,           // x
                         meterBounds.getBottom(),      // y
                         meterWidth,                   // width
                         12,                           // height
                         juce::Justification::centred, // justification
                         1);                           // num lines
    }
}

void TriMeter::resized()
{
    ticks.clear();
    ticks = getTicks(6);
}

void TriMeter::update(float inDb, float outDb)
{
    inValueDb = inDb;
    outValueDb = outDb;
    repaint();
}

std::vector<Tick> TriMeter::getTicks(int dbDivision)
{
    auto maxDb = Globals::getMaxDecibels();
    auto minDb = Globals::getNegativeInf();

    auto numTicks = (maxDb - minDb) / dbDivision;
        
    std::vector<Tick> ticks;
    ticks.reserve(numTicks);
    
    auto meterBounds = getMeterBounds();
    
    auto meterHeight = meterBounds.getHeight();
    
    for ( auto db = minDb; db <= maxDb; db += dbDivision )
    {
        Tick tick;
        tick.db = db;
        tick.y = juce::jmap<int>(db, Globals::getNegativeInf(), Globals::getMaxDecibels(), meterHeight, 0);
        ticks.emplace_back(tick);
    }
    
    return ticks;
}

juce::Rectangle<int> TriMeter::getMeterBounds()
{
    return getLocalBounds().reduced(0, 16);
}
