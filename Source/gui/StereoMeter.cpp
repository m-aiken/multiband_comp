/*
  ==============================================================================

    StereoMeter.cpp
    Created: 31 May 2022 1:07:26am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "StereoMeter.h"
#include "../Globals.h"

//==============================================================================
StereoMeter::StereoMeter()
{
    addAndMakeVisible(meterL);
    addAndMakeVisible(meterR);
    addAndMakeVisible(dbScale);
}

void StereoMeter::resized()
{
    auto bounds = getLocalBounds();
    auto padding = bounds.getHeight() / 20;
    auto widthUnit = bounds.getWidth() / 7;
    
    meterL.setBounds(0,                                   // x
                     padding,                             // y
                     widthUnit * 2,                       // width
                     bounds.getHeight() - (padding * 2)); // height
    
    dbScale.setBounds(meterL.getRight(),
                      0,
                      widthUnit * 3,
                      bounds.getHeight());
    
    meterR.setBounds(dbScale.getRight(),
                     padding,
                     widthUnit * 2,
                     bounds.getHeight() - (padding * 2));
    
    /*
    calculate the actual meter bounds so the db scale isn't mapped to a range that includes the label
    */
    auto actualMeterHeight = meterL.getHeight() * (meterL.getMeterProportion() / 100); // to remove the label bounds
    auto meterYOffset = meterL.getHeight() - actualMeterHeight;
    auto meterBoundsForDbScale = juce::Rectangle<int>(meterL.getBounds()).withHeight(actualMeterHeight).withY(meterL.getY() + meterYOffset);
    dbScale.buildBackgroundImage(6, meterBoundsForDbScale, Globals::negativeInf(), Globals::maxDecibels());
}

void StereoMeter::update(const MeterValues& meterValues)
{
    meterL.update(meterValues.leftPeakDb.getDb(), meterValues.leftRmsDb.getDb());
    meterR.update(meterValues.rightPeakDb.getDb(), meterValues.rightRmsDb.getDb());
}
