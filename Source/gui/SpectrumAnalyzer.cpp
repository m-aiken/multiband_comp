/*
  ==============================================================================

    SpectrumAnalyzer.cpp
    Created: 23 Jul 2022 12:41:32pm
    Author:  Matt Aiken

  ==============================================================================
*/

#include "SpectrumAnalyzer.h"
#include "../dsp/AnalyzerProperties.h"
#include "../ColourPalette.h"

//==============================================================================
SpectrumAnalyzer::SpectrumAnalyzer(double _sampleRate,
                                   SingleChannelSampleFifo<juce::AudioBuffer<float>>& leftScsf,
                                   SingleChannelSampleFifo<juce::AudioBuffer<float>>& rightScsf,
                                   juce::AudioProcessorValueTreeState& apvts)
: sampleRate(_sampleRate),
  leftPathProducer(_sampleRate, leftScsf),
  rightPathProducer(_sampleRate, rightScsf)
{
    const auto& params = AnalyzerProperties::getAnalyzerParams();
    
    auto initListener = [&](auto& listener, const auto& paramName, const auto& lambda)
    {
        auto param = apvts.getParameter(params.at(paramName));
        jassert(param != nullptr);
        listener = std::make_unique<ParamListener<float>>(*param, lambda);
    };
    
    initListener(analyzerEnabledParamListener,
                 AnalyzerProperties::ParamNames::Enable_Analyzer,
                 [this](const auto& activeStatus){ setActive(activeStatus); });
    
    initListener(analyzerDecayRateParamListener,
                 AnalyzerProperties::ParamNames::Analyzer_Decay_Rate,
                 [this](const auto& decayRate){ updateDecayRate(decayRate); });
    
    initListener(analyzerOrderParamListener,
                 AnalyzerProperties::ParamNames::Analyzer_Points,
                 [this](const auto& newOrder){ updateOrder(newOrder); });
    
    auto enabledParam = apvts.getParameter(params.at(AnalyzerProperties::ParamNames::Enable_Analyzer));
    setActive(enabledParam->getValue());

    auto decayRateParam = apvts.getParameter(params.at(AnalyzerProperties::ParamNames::Analyzer_Decay_Rate));
    updateDecayRate(decayRateParam->convertFrom0to1(decayRateParam->getValue()));

    auto orderParam = apvts.getParameter(params.at(AnalyzerProperties::ParamNames::Analyzer_Points));
    updateOrder(orderParam->getValue());
    
    leftPathProducer.toggleProcessing(true);
    rightPathProducer.toggleProcessing(true);
    
    addAndMakeVisible(analyzerScale);
    addAndMakeVisible(eqScale);
    
    animate();
}

void SpectrumAnalyzer::timerCallback()
{
    if (!active)
    {
        leftAnalyzerPath.clear();
        rightAnalyzerPath.clear();
        stopTimer();
    }
    else
    {
        if ( leftPathProducer.getNumAvailableForReading() > 0 )
        {
            while ( leftPathProducer.pull(leftAnalyzerPath) ) { } // get most recent
        }
        
        if ( rightPathProducer.getNumAvailableForReading() > 0 )
        {
            while ( rightPathProducer.pull(rightAnalyzerPath) ) { } // get most recent
        }
    }
    repaint();
}

void SpectrumAnalyzer::resized()
{
    AnalyzerBase::resized();
    leftPathProducer.setFFTRectBounds(fftBoundingBox.toFloat());
    rightPathProducer.setFFTRectBounds(fftBoundingBox.toFloat());
    
    auto textHeight = getTextHeight();
    auto textWidth = getTextWidth() * 1.5;
    
    auto localBounds = getLocalBounds();
    auto fftBoundsY = fftBoundingBox.getY();
    auto fftBoundsHeight = fftBoundingBox.getHeight();
    
    analyzerScale.setBounds(localBounds.getX(),
                            fftBoundsY - getTextOffset(),
                            textWidth,
                            fftBoundsHeight + textHeight);
    
    eqScale.setBounds(localBounds.getRight() - textWidth,
                      fftBoundsY - getTextOffset(),
                      textWidth,
                      fftBoundsHeight + textHeight);
    
    customizeScales(leftScaleMin, leftScaleMax, rightScaleMin, rightScaleMax, scaleDivision);
}

void SpectrumAnalyzer::paint(juce::Graphics& g)
{
    paintBackground(g);
    
    g.reduceClipRegion(fftBoundingBox);
    
    leftAnalyzerPath.applyTransform(juce::AffineTransform().translation(fftBoundingBox.getX(), fftBoundingBox.getY() - getTextOffset()));
    rightAnalyzerPath.applyTransform(juce::AffineTransform().translation(fftBoundingBox.getX(), fftBoundingBox.getY() - getTextOffset()));
    
    g.setColour(juce::Colours::lightblue);
    g.strokePath(leftAnalyzerPath, juce::PathStrokeType(1.f));
    
    g.setColour(juce::Colours::lightgreen);
    g.strokePath(rightAnalyzerPath, juce::PathStrokeType(1.f));
}

void SpectrumAnalyzer::customizeScales(int lsMin, int lsMax, int rsMin, int rsMax, int division)
{
    leftScaleMin = lsMin;
    leftScaleMax = lsMax;
    rightScaleMin = rsMin;
    rightScaleMax = rsMax;
    
    leftPathProducer.changePathRange(leftScaleMin, leftScaleMax);
    rightPathProducer.changePathRange(leftScaleMin, leftScaleMax);
    
    analyzerScale.buildBackgroundImage(division, fftBoundingBox, leftScaleMin, leftScaleMax);
    eqScale.buildBackgroundImage(division, fftBoundingBox, leftScaleMin, leftScaleMax);
    
    if (!getLocalBounds().isEmpty())
    {
        repaint();
    }
}

void SpectrumAnalyzer::paintBackground(juce::Graphics& g)
{
    const float minFreq = Globals::getMinFrequency();
    const float maxFreq = Globals::getMaxFrequency();
    auto fftBoundsX = fftBoundingBox.getX();
    auto fftBoundsY = fftBoundingBox.getY();
    auto fftBoundsWidth = fftBoundingBox.getWidth();
    auto fftBoundsBottom = fftBoundingBox.getBottom();
    auto fftBoundsRight = fftBoundingBox.getRight();
    auto textWidth = AnalyzerBase::getTextWidth();
    auto textHeight = AnalyzerBase::getTextHeight();
    
    g.setColour(ColourPalette::getColour(ColourPalette::Text).withLightness(0.2f));
    
    // dB markers
    for ( auto i = leftScaleMin; i <= leftScaleMax; i += scaleDivision )
    {
        auto yCoord = juce::jmap<float>(i, leftScaleMin, leftScaleMax, fftBoundsBottom, fftBoundsY);
        g.drawLine(fftBoundsX + 2, yCoord, fftBoundsRight - 2, yCoord, 1.f);
    }
    
    // Frequency markers
    std::vector<float> freqs
    {
        20.f, 50.f, 100.f,
        200.f, 500.f, 1000.f,
        2000.f, 5000.f, 10000.f,
        20000.f
    };
    
    for ( auto i = 1; i < freqs.size() - 1; ++i )
    {
        auto normalizedX = juce::mapFromLog10<float>(freqs[i], minFreq, maxFreq);
        auto freqLineX = fftBoundsX + fftBoundsWidth * normalizedX;
        
        g.setColour(ColourPalette::getColour(ColourPalette::Text).withLightness(0.2f));
        g.drawVerticalLine(freqLineX, fftBoundsY, fftBoundsBottom);
        
        juce::String text = freqs[i] >= 1000.f ? juce::String(freqs[i] / 1000.f) + "kHz" : juce::String(freqs[i]) + "Hz";
        
        g.setColour(ColourPalette::getColour(ColourPalette::Text));
        g.drawFittedText(text,                                     // text
                         freqLineX - (textWidth * 1.5),            // x
                         fftBoundsY + (textHeight * 1.5),          // y
                         textWidth * 3,                            // width
                         textHeight,                               // height
                         juce::Justification::horizontallyCentred, // justification
                         1);                                       // max num lines
    }
}

void SpectrumAnalyzer::setActive(bool activeState)
{
    active = activeState;
    if (active && !isTimerRunning())
    {
        animate();
    }
}

void SpectrumAnalyzer::updateDecayRate(float decayRate)
{
    leftPathProducer.setDecayRate(decayRate);
    rightPathProducer.setDecayRate(decayRate);
}

void SpectrumAnalyzer::updateOrder(float value)
{
    auto denormalizedVal = static_cast<int>(std::floor(juce::jmap<float>(value, 0.f, 1.f, 11.f, 13.f)));
    leftPathProducer.changeOrder(static_cast<FFTOrder>(denormalizedVal));
    rightPathProducer.changeOrder(static_cast<FFTOrder>(denormalizedVal));
}

void SpectrumAnalyzer::animate()
{
    startTimerHz(60);
}
