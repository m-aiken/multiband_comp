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

void LookAndFeel::drawLinearSlider(juce::Graphics& g,
                                   int x, int y, int width, int height,
                                   float sliderPos, float minSliderPos, float maxSliderPos,
                                   const juce::Slider::SliderStyle sliderStyle,
                                   juce::Slider& slider)
{
    auto tooltipHeight = height * 0.15;
    float tooltipY = sliderPos - (tooltipHeight * 0.5);
    
    if (tooltipY < y)
    {
        tooltipY = y;
    }
    
    if (height - sliderPos < tooltipHeight)
    {
        tooltipY = height - tooltipHeight;
    }
    
    auto tooltip = juce::Rectangle<float>(0.f, tooltipY, width, tooltipHeight);
    g.setColour(juce::Colours::skyblue);
    g.fillRect(tooltip.reduced(2.f, 2.f));
}

void LookAndFeel::drawToggleButton(juce::Graphics& g,
                                   juce::ToggleButton& toggleButton,
                                   bool shouldDrawButtonAsHighlighted,
                                   bool shouldDrawButtonAsDown)
{
    auto bounds = toggleButton.getLocalBounds();
    
    // background
    juce::Colour backgroundColour;
    
    if (shouldDrawButtonAsDown)
    {
        backgroundColour = ColourPalette::getColour(ColourPalette::Background);
    }
    else
    {
        backgroundColour = shouldDrawButtonAsHighlighted ? juce::Colours::skyblue.brighter() : juce::Colours::skyblue;
    }
    
    g.fillAll(backgroundColour);
    
    // text colour
    g.setColour(shouldDrawButtonAsDown
                ? ColourPalette::getColour(ColourPalette::Text)
                : ColourPalette::getColour(ColourPalette::Background));
    
    g.drawFittedText(shouldDrawButtonAsDown ? "Off" : "On",
                     bounds.getX(),
                     bounds.getY(),
                     bounds.getWidth(),
                     bounds.getHeight(),
                     juce::Justification::centred,
                     1);
}
