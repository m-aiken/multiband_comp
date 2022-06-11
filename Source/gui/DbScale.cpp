/*
  ==============================================================================

    DbScale.cpp
    Created: 31 May 2022 1:01:45am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "DbScale.h"
#include "../Globals.h"
#include "../ColourPalette.h"

//==============================================================================
void DbScale::paint(juce::Graphics& g)
{
    g.drawImage(bkgd, getLocalBounds().toFloat());
}

void DbScale::buildBackgroundImage(int dbDivision, juce::Rectangle<int> meterBounds, int minDb, int maxDb)
{
    jassert( minDb < maxDb );
    
    auto bounds = getLocalBounds();
    if ( bounds.isEmpty() )
        return;
    
    auto scaleFactor = juce::Desktop::getInstance().getGlobalScaleFactor();
    
    bkgd = juce::Image(juce::Image::PixelFormat::RGB,
                       bounds.getWidth() * scaleFactor,
                       bounds.getHeight() * scaleFactor,
                       true);
    
    juce::Graphics g(bkgd);
    g.addTransform(juce::AffineTransform::scale(scaleFactor));
    
    auto ticks = getTicks(dbDivision, meterBounds, minDb, maxDb);
    auto boundsX = bounds.getX();
    auto boundsWidth = bounds.getWidth();
    auto textHeight = 12;
    auto meterY = meterBounds.getY();
    
    g.setColour(ColourPalette::getColour(ColourPalette::Text));
    
    for ( auto i = 0; i < ticks.size(); ++i )
    {
        auto dbString = juce::String(ticks[i].db);
        
        g.drawFittedText((ticks[i].db > 0 ? '+' + dbString : dbString), // text
                         boundsX,                                       // x
                         ticks[i].y + meterY - (textHeight / 2),        // y
                         boundsWidth,                                   // width
                         textHeight,                                    // height
                         juce::Justification::centred,                  // justification
                         1);                                            // max num lines
    }
}

std::vector<Tick> DbScale::getTicks(int dbDivision, juce::Rectangle<int> meterBounds, int minDb, int maxDb)
{
    if ( minDb > maxDb )
    {
        std::swap(minDb, maxDb);
    }
    
    auto numTicks = (maxDb - minDb) / dbDivision;
    
    std::vector<Tick> ticks;
    ticks.reserve(numTicks);
    
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
