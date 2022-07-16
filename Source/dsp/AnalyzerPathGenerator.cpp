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
    
    const float minFreq = Globals::getMinFrequency();
    const float maxFreq = Globals::getMaxFrequency();
    
    auto fftBoundsBottom = fftBounds.getBottom();
    auto fftBoundsY = fftBounds.getY();
    auto fftBoundsWidth = fftBounds.getWidth();
    
    juce::Path path;
    path.preallocateSpace(3 * static_cast<int>(fftBoundsWidth));

    auto yCoord = juce::jmap<float>(renderData[0], negativeInfinity, maxDb, fftBoundsBottom, fftBoundsY);
    if ( std::isnan(yCoord) || std::isinf(yCoord) )
    {
        yCoord = fftBoundsBottom;
    }
    
    path.startNewSubPath(0, yCoord);
    
    auto lastXCoord = fftBounds.getX();
    
    const int pathResolution = 2;
    
    for ( auto i = 1; i < numBins + 1; i += pathResolution )
    {
        yCoord = juce::jmap<float>(renderData[i], negativeInfinity, maxDb, fftBoundsBottom, fftBoundsY);

        if ( !std::isnan(yCoord) && !std::isinf(yCoord) )
        {
            auto binFreq = i * binWidth;
            auto normalizedX = juce::mapFromLog10(binFreq, minFreq, maxFreq);
            auto xCoord = std::floor(normalizedX * fftBoundsWidth);

            if ( xCoord - lastXCoord >= pathResolution && xCoord <= fftBoundsWidth )
            {
                path.lineTo(xCoord, yCoord);
                lastXCoord = xCoord;
            }
        }
    }
    
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
