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
    MeterYellow,
    MeterBackground,
    Blue,
    Border
};

inline std::map<Palette, juce::Colour> colourMap =
{
    { Background,      juce::Colour(46u, 41u, 43u) },
    { Text,            juce::Colour(201u, 209u, 207u) },
    { Red,             juce::Colour(219u, 66u, 65u) },
    { MeterGreen,      juce::Colour(147u, 236u, 148u) },
    { MeterYellow,     juce::Colour(253u, 158u, 43u).withAlpha(0.75f) },
    { MeterBackground, juce::Colour(37u, 32u, 37u) },
    { Blue,            juce::Colour(174u, 216u, 229u) },
    { Border,          juce::Colour(201u, 209u, 207u).withAlpha(0.25f) }
};

inline juce::Colour getColour(Palette c) { return colourMap[c]; }

}
