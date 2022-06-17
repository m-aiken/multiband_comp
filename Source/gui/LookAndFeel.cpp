/*
  ==============================================================================

    LookAndFeel.cpp
    Created: 1 Jun 2022 12:09:57am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "LookAndFeel.h"
#include "../ColourPalette.h"

//==============================================================================
void LookAndFeel::drawRotarySlider(juce::Graphics& g,
                                         int x, int y, int width, int height,
                                         float sliderPosProportional,
                                         float rotaryStartAngle,
                                         float rotaryEndAngle,
                                         juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float>(x, y, width, height);
    
    g.setColour(ColourPalette::getColour(ColourPalette::Background).contrasting(0.05f));
    g.fillEllipse(bounds);
    g.setColour(juce::Colours::skyblue);
    g.drawEllipse(bounds, 1.f);
    
    g.setColour(ColourPalette::getColour(ColourPalette::Text));
    
    juce::Path p;
    
    auto centre = bounds.getCentre();
    
    juce::Rectangle<float> r;
    r.setLeft(centre.getX() - 2);
    r.setRight(centre.getX() + 2);
    r.setTop(bounds.getY() + 4);
    r.setBottom(bounds.getY() + 20);
    
    p.addRoundedRectangle(r, 2.f);
    
    jassert(rotaryStartAngle < rotaryEndAngle);
    
    auto transform = juce::AffineTransform().rotated(sliderPosProportional,
                                                     centre.getX(),
                                                     centre.getY());
    
    p.applyTransform(transform);
    
    g.fillPath(p);
}
