/*
  ==============================================================================

    CompressorBandControl.cpp
    Created: 1 Jun 2022 12:00:35am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "CompressorBandControl.h"
#include "../Params.h"

//==============================================================================
CompressorBandControl::CompressorBandControl(juce::AudioProcessorValueTreeState& _apvts)
    : apvts(_apvts)
{
    auto initRotaryControl = [&apvts = this->apvts](auto& rotaryControl, const auto& paramName, const auto& suffix, const auto& bandNum)
    {
        auto param = apvts.getParameter(Params::getBandControlParamName(paramName, bandNum));
        jassert( param != nullptr );
        
        rotaryControl = std::make_unique<RotaryControl>(*param, suffix, Params::bandControlMap.at(paramName));
    };
    
    initRotaryControl(attackRotary,     Params::BandControl::Attack,    "ms", 0);
    initRotaryControl(releaseRotary,    Params::BandControl::Release,   "ms", 0);
    initRotaryControl(thresholdRotary,  Params::BandControl::Threshold, "dB", 0);
    initRotaryControl(makeupGainRotary, Params::BandControl::Gain,      "dB", 0);
    initRotaryControl(ratioRotary,      Params::BandControl::Ratio,     "",   0);
    
    attackAttachment     = std::make_unique<Attachment>(apvts, Params::getBandControlParamName(Params::BandControl::Attack, 0),    *attackRotary);
    releaseAttachment    = std::make_unique<Attachment>(apvts, Params::getBandControlParamName(Params::BandControl::Release, 0),   *releaseRotary);
    thresholdAttachment  = std::make_unique<Attachment>(apvts, Params::getBandControlParamName(Params::BandControl::Threshold, 0), *thresholdRotary);
    makeupGainAttachment = std::make_unique<Attachment>(apvts, Params::getBandControlParamName(Params::BandControl::Gain, 0),      *makeupGainRotary);
    ratioAttachment      = std::make_unique<Attachment>(apvts, Params::getBandControlParamName(Params::BandControl::Ratio, 0),     *ratioRotary);
    
    resetButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::red);
    resetButton.setColour(juce::TextButton::ColourIds::textColourOffId, juce::Colours::white);
    resetButton.onClick = [this]() { resetParamsToDefault(); };
    
    addAndMakeVisible(*attackRotary);
    addAndMakeVisible(*releaseRotary);
    addAndMakeVisible(*thresholdRotary);
    addAndMakeVisible(*makeupGainRotary);
    addAndMakeVisible(*ratioRotary);
    
    addAndMakeVisible(resetButton);
}

void CompressorBandControl::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::black);
    g.drawRect(getLocalBounds());
}

void CompressorBandControl::resized()
{
    auto bounds = getLocalBounds();
    auto rotaryBounds = bounds.withWidth(bounds.getWidth() * 0.9);

    // Rotaries
    juce::Grid grid;
    
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    
    grid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
    grid.autoRows = Track(Fr(1));
    
    grid.items =
    {
        juce::GridItem(*attackRotary),
        juce::GridItem(*releaseRotary),
        juce::GridItem(*thresholdRotary),
        juce::GridItem(*makeupGainRotary),
        juce::GridItem(*ratioRotary)
    };
    
    grid.performLayout(rotaryBounds);
    
    // Button
    auto buttonSize = bounds.getWidth() * 0.06;
    resetButton.setBounds(bounds.getRight() - buttonSize - 5,
                          bounds.getCentreY() - (buttonSize * 0.5),
                          buttonSize,
                          buttonSize);
}

void CompressorBandControl::resetHelper(const juce::String& paramName)
{
    auto param = apvts.getParameter(paramName);
    jassert( param != nullptr );
    
    resetHelper(param, param->getDefaultValue());
}

void CompressorBandControl::resetHelper(juce::RangedAudioParameter* param, const float& newValue)
{
    param->beginChangeGesture();
    param->setValueNotifyingHost(newValue);
    param->endChangeGesture();
}

void CompressorBandControl::resetParamsToDefault()
{
    resetHelper(Params::getBandControlParamName(Params::BandControl::Attack, 0));
    resetHelper(Params::getBandControlParamName(Params::BandControl::Release, 0));
    resetHelper(Params::getBandControlParamName(Params::BandControl::Threshold, 0));
    resetHelper(Params::getBandControlParamName(Params::BandControl::Gain, 0));
    resetHelper(Params::getBandControlParamName(Params::BandControl::Ratio, 0));
}
