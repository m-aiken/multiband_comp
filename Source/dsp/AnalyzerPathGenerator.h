/*
  ==============================================================================

    AnalyzerPathGenerator.h
    Created: 9 Jul 2022 12:24:20am
    Author:  Matt Aiken

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "../Globals.h"
#include "Fifo.h"

//==============================================================================
struct AnalyzerPathGenerator
{
    void generatePath(const std::vector<float>& renderData,
                      juce::Rectangle<float> fftBounds,
                      int fftSize,
                      float binWidth,
                      float negativeInfinity = Globals::getNegativeInf(),
                      float maxDb = Globals::getMaxDecibels());
    
    int getNumPathsAvailable() const;
    bool getPath(juce::Path& path);
private:
    Fifo<juce::Path, 20> pathFifo;
};
