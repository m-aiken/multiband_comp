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
Button::Button(const juce::String& buttonText)
{
    setLookAndFeel(&lnf);
    setButtonText(buttonText);
}

void Button::paint(juce::Graphics& g)
{
    getLookAndFeel().drawButtonBackground(g,
                                          *this,              // button
                                          juce::Colours::red, // colour
                                          true,               // draw as highlighted
                                          false);             // draw as down
}

//==============================================================================
CompressorBandControl::CompressorBandControl(juce::AudioProcessorValueTreeState& _apvts)
    : apvts(_apvts)
{
    auto initRotaryControl = [&apvts = this->apvts](auto& rotaryControl, const auto& paramName, const auto& suffix, const auto& title)
    {
        auto param = apvts.getParameter(paramName);
        jassert( param != nullptr );
        
        rotaryControl = std::make_unique<RotaryControl>(*param, suffix, title);
    };
    
    initRotaryControl(attackRotary,     Params::getAttackParamName(0),    "ms",     "ATTACK");
    initRotaryControl(releaseRotary,    Params::getReleaseParamName(0),   "ms",     "RELEASE");
    initRotaryControl(thresholdRotary,  Params::getThresholdParamName(0), "dB",     "THRESHOLD");
    initRotaryControl(makeupGainRotary, Params::getGainParamName(0),      "dB",     "GAIN");
    initRotaryControl(ratioRotary,      Params::getRatioParamName(0),     "",       "RATIO");
    
    attackAttachment     = std::make_unique<Attachment>(apvts, Params::getAttackParamName(0),    *attackRotary);
    releaseAttachment    = std::make_unique<Attachment>(apvts, Params::getReleaseParamName(0),   *releaseRotary);
    thresholdAttachment  = std::make_unique<Attachment>(apvts, Params::getThresholdParamName(0), *thresholdRotary);
    makeupGainAttachment = std::make_unique<Attachment>(apvts, Params::getGainParamName(0),      *makeupGainRotary);
    ratioAttachment      = std::make_unique<Attachment>(apvts, Params::getRatioParamName(0),     *ratioRotary);
    
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
    resetHelper(Params::getAttackParamName(0));
    resetHelper(Params::getReleaseParamName(0));
    resetHelper(Params::getThresholdParamName(0));
    resetHelper(Params::getGainParamName(0));
    resetHelper(Params::getRatioParamName(0));
}
