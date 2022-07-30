/*
  ==============================================================================

    LinearSlider.cpp
    Created: 30 Jul 2022 3:12:05pm
    Author:  Matt Aiken

  ==============================================================================
*/

#include "LinearSlider.h"
#include "../ColourPalette.h"

//==============================================================================
CustomLinearSlider::CustomLinearSlider(juce::RangedAudioParameter& rap)
: juce::Slider(juce::Slider::SliderStyle::LinearVertical, juce::Slider::TextEntryBoxPosition::NoTextBox),
  param(&rap)
{
    setLookAndFeel(&lnf);
}

void CustomLinearSlider::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.setColour(ColourPalette::getColour(ColourPalette::Text));
    g.drawRect(bounds);
    
    auto paramNormalisableRange = param->getNormalisableRange();
    auto scaledValue = juce::jmap<float>(param->convertFrom0to1(param->getValue()), paramNormalisableRange.start, paramNormalisableRange.end, bounds.getBottom(), bounds.getY());
    getLookAndFeel().drawLinearSlider(g,                                         // graphics
                                      bounds.getX(),                             // x
                                      bounds.getY(),                             // y
                                      bounds.getWidth(),                         // width
                                      bounds.getHeight(),                        // height
                                      scaledValue,                               // slider position
                                      bounds.getBottom(),                        // min slider position
                                      bounds.getY(),                             // max slider position
                                      juce::Slider::SliderStyle::LinearVertical, // slider style
                                      *this);                                    // slider
}
