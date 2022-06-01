/*
  ==============================================================================

    RotaryControl.cpp
    Created: 1 Jun 2022 12:30:34am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "RotaryControl.h"

//==============================================================================
void RotaryControl::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto diameter = bounds.getWidth() * 0.7f;
    auto radius = diameter / 2;
    
    auto startAngle = juce::degreesToRadians(180.f + 45.f);
    auto endAngle = juce::degreesToRadians(180.f - 45.f) + juce::MathConstants<float>::twoPi;
    
    auto range = getRange();
    
    auto valueToDraw = juce::jmap<float>(getValue(),
                                         range.getStart(),
                                         range.getEnd(),
                                         startAngle,
                                         endAngle);
    
    getLookAndFeel().drawRotarySlider(g,
                                      bounds.getCentreX() - radius, // x
                                      bounds.getCentreY() - radius, // y
                                      diameter,                     // width
                                      diameter,                     // height
                                      valueToDraw,                  // position
                                      startAngle,                   // start angle
                                      endAngle,                     // end angle
                                      *this);                       // slider
}
