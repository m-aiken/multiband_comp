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
void CustomToggleButton::paint(juce::Graphics& g)
{
    getLookAndFeel().drawToggleButton(g,                  // graphics
                                      *this,              // button
                                      isMouseOver(),      // draw as highlighted
                                      !getToggleState()); // draw as down
    
    g.setColour(ColourPalette::getColour(ColourPalette::Text));
    g.drawRect(getLocalBounds());
}
