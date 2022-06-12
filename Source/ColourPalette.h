/*
  ==============================================================================

    ColourPalette.h
    Created: 11 Jun 2022 1:18:07pm
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
namespace ColourPalette
{

enum Palette
{
    Background,
    Text,
    Red,
    MeterGreen,
    MeterYellow
};

inline std::map<Palette, juce::Colour> colourMap =
{
    { Background,  juce::Colour(0u, 18u, 25u)     },
    { Text,        juce::Colour(201u, 209u, 207u) },
    { Red,         juce::Colour(255u, 34u, 44u)   },
    { MeterGreen,  juce::Colour(82u, 182u, 154u)  },
    { MeterYellow, juce::Colour(217u, 237u, 146u) }
};

inline juce::Colour getColour(Palette c) { return colourMap[c]; }

}
