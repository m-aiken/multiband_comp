/*
  ==============================================================================

    CustomToggleButton.cpp
    Created: 30 Jul 2022 5:36:51pm
    Author:  Matt Aiken

  ==============================================================================
*/

#include "CustomToggleButton.h"
#include "../ColourPalette.h"

//==============================================================================
CustomToggleButton::CustomToggleButton(juce::RangedAudioParameter& rap)
: param(&rap)
{
    setLookAndFeel(&lnf);
}

void CustomToggleButton::paint(juce::Graphics& g)
{
    getLookAndFeel().drawToggleButton(g,      // graphics
                                      *this,  // button
                                      false,  // draw as highlighted
                                      false); // draw as down
    
    g.setColour(ColourPalette::getColour(ColourPalette::Text));
    g.drawRect(getLocalBounds());
}
