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
    
    g.setColour(ColourPalette::getColour(ColourPalette::Background).contrasting(0.05f));
    g.fillRect(meterBounds);
    
    // meters
    // in meter
    g.setColour(juce::Colours::skyblue);
    auto inDbScaled = std::floor(juce::jmap<float>(inValueDb, Globals::getNegativeInf(), Globals::getMaxDecibels(), meterHeight, 0));
    auto inMeter = juce::Rectangle<int>(0, inDbScaled, meterWidth, meterHeight - inDbScaled);
    g.fillRect(inMeter);
    
    // out meter
    g.setColour(ColourPalette::getColour(ColourPalette::MeterGreen));
    auto outDbScaled = std::floor(juce::jmap<float>(outValueDb, Globals::getNegativeInf(), Globals::getMaxDecibels(), meterHeight, 0));
    auto outMeter = juce::Rectangle<int>(meterWidth, outDbScaled, meterWidth, meterHeight - outDbScaled);
    g.fillRect(outMeter);

    // gain reduction
    auto gainReduction = std::floor(juce::jmap<float>(outValueDb - inValueDb, Globals::getNegativeInf(), Globals::getMaxDecibels(), meterHeight, 0));
    auto grMeter = juce::Rectangle<int>(meterWidth * 2, 0, meterWidth, gainReduction);
    grMeter.removeFromTop(zeroDbTick.y);
    g.fillRect(grMeter);
    
    // ticks
    for ( auto i = 0; i < 3; ++i )
    {
        g.setColour(ColourPalette::getColour(ColourPalette::Background));
        for ( auto j = 1; j < ticks.size() - 1; ++j ) // excluding first and last - they should be covered up by the bounding box anyway
        {
            if (ticks[j].y != zeroDbTick.y)
            {
                auto indent = j % 2 == 0 ? 4 : 8;
                g.drawLine((meterWidth * i) + indent,     // start x
                           ticks[j].y,                    // start y
                           (meterWidth * (i+1)) - indent, // end x
                           ticks[j].y,                    // end y
                           1.f);                          // line thickness
            }
        }
        // draw the zero dB line separately so we're not reseting the colour for each tick
        g.setColour(ColourPalette::getColour(ColourPalette::MeterGreen));
        g.drawLine((meterWidth * i) + 4,     // start x
                   zeroDbTick.y,             // start y
                   (meterWidth * (i+1)) - 4, // end x
                   zeroDbTick.y,             // end y
                   1.f);                     // line thickness
    }
    
    // separation lines
    g.setColour(ColourPalette::getColour(ColourPalette::Background));
    g.drawLine(meterWidth * 1, 1, meterWidth * 1, meterBounds.getBottom() - 1, 1.f);
    g.drawLine(meterWidth * 2, 1, meterWidth * 2, meterBounds.getBottom() - 1, 1.f);
    
    g.setColour(ColourPalette::getColour(ColourPalette::Text));
    g.drawRect(meterBounds);
    
    // labels
    std::array<juce::String, 3> labels = { "I", "O", "GR" };
    for ( auto label = 0; label < labels.size(); ++label )
    {
        g.drawFittedText(labels[label],                // text
                         meterWidth * label,           // x
                         meterBounds.getBottom(),      // y
                         meterWidth,                   // width
                         16,                           // height
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
        
        if (db == 0.f)
        {
            // store this so we don't have to recompute the 0db y location on each repaint
            zeroDbTick = tick;
        }
    }
    
    return ticks;
}

juce::Rectangle<int> TriMeter::getMeterBounds()
{
    auto bounds = getLocalBounds();
    bounds.removeFromBottom(16);
    return bounds;
}
