/*
  ==============================================================================

    AnalyzerPathGenerator.cpp
    Created: 9 Jul 2022 12:24:20am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "AnalyzerPathGenerator.h"

//==============================================================================
void AnalyzerPathGenerator::generatePath(const std::vector<float>& renderData,
                                         juce::Rectangle<float> fftBounds,
                                         int fftSize,
                                         float binWidth,
                                         float negativeInfinity,
                                         float maxDb)
{
    auto numBins = static_cast<int>(fftSize * 0.5);
    
    juce::Path path;
    path.startNewSubPath(juce::Point<float>(0.f, fftBounds.getBottom()));
    
    const int pathResolution = 2;
    
    for ( auto i = 1; i < numBins + 1; i += pathResolution )
    {
        auto yCoord = juce::jmap<float>(renderData[i], negativeInfinity, maxDb, fftBounds.getBottom(), fftBounds.getY());
        
        if (!std::isnan(yCoord) && !std::isinf(yCoord))
        {
            auto binFreq = i * binWidth;
            auto normalizedX = juce::mapFromLog10(binFreq, Globals::getMinFrequency(), Globals::getMaxFrequency());
            auto xCoord = std::floor(normalizedX * fftBounds.getWidth());
            path.lineTo(juce::Point<float>(xCoord, yCoord));
        }
    }
    
    path.closeSubPath();
    
    pathFifo.push(path);
}

int AnalyzerPathGenerator::getNumPathsAvailable() const
{
    return pathFifo.getNumAvailableForReading();
}

bool AnalyzerPathGenerator::getPath(juce::Path& path)
{
    return pathFifo.pull(path);
}
