/*
  ==============================================================================

    AnalyzerBase.h
    Created: 23 Jul 2022 12:27:02am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
struct AnalyzerBase : juce::Component
{
    juce::Rectangle<int> getBoundsForRendering() const
    {
        return getLocalBounds().reduced(0, static_cast<int>(std::floor(getTextHeight() * 0.6)));
    }
    
    juce::Rectangle<int> getBoundsForFFT()
    {
        auto boundsForRendering = getBoundsForRendering();
        auto w = getTextWidth() * 1.5;
        return boundsForRendering.withTrimmedLeft(w).withTrimmedRight(w);
    }
    
    inline int getTextHeight() const { return 12; }
    inline int getTextWidth() const { return 16; }
    void resized() override
    {
        fftBoundingBox = getBoundsForFFT();
    }
protected:
    juce::Rectangle<int> fftBoundingBox;
};
