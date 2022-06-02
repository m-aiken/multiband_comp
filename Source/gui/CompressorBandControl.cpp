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
                                          *this,                   // button
                                          juce::Colours::darkgrey, // colour
                                          true,                    // draw as highlighted
                                          false);                  // draw as down
}

//==============================================================================
CompressorBandControl::CompressorBandControl(juce::AudioProcessorValueTreeState& _apvts)
    : apvts(_apvts)
{
    auto initRotaryControl = [&apvts = this->apvts](auto& rotaryControl, const auto& paramName, const auto& suffix)
    {
        auto param = apvts.getParameter(paramName);
        jassert( param != nullptr );
        
        rotaryControl = std::make_unique<RotaryControl>(*param, suffix);
    };
    
    initRotaryControl(attackRotary,     Params::getAttackParamName(0),    "ms");
    initRotaryControl(releaseRotary,    Params::getReleaseParamName(0),   "ms");
    initRotaryControl(thresholdRotary,  Params::getThresholdParamName(0), "dB");
    initRotaryControl(makeupGainRotary, Params::getGainParamName(0),      "dB");
    initRotaryControl(ratioRotary,      Params::getRatioParamName(0),     "db/Sec");
    
    attackAttachment     = std::make_unique<Attachment>(apvts, Params::getAttackParamName(0),    *attackRotary);
    releaseAttachment    = std::make_unique<Attachment>(apvts, Params::getReleaseParamName(0),   *releaseRotary);
    thresholdAttachment  = std::make_unique<Attachment>(apvts, Params::getThresholdParamName(0), *thresholdRotary);
    makeupGainAttachment = std::make_unique<Attachment>(apvts, Params::getGainParamName(0),      *makeupGainRotary);
    ratioAttachment      = std::make_unique<Attachment>(apvts, Params::getRatioParamName(0),     *ratioRotary);
    
    addAndMakeVisible(*attackRotary);
    addAndMakeVisible(*releaseRotary);
    addAndMakeVisible(*thresholdRotary);
    addAndMakeVisible(*makeupGainRotary);
    addAndMakeVisible(*ratioRotary);
    
    addAndMakeVisible(resetButton);
}

void CompressorBandControl::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::yellow);
}

void CompressorBandControl::resized()
{
    juce::Grid grid;
    
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    
    auto rotaryFr = 3;
    
    grid.templateColumns =
    {
        Track(Fr(rotaryFr)),
        Track(Fr(rotaryFr)),
        Track(Fr(rotaryFr)),
        Track(Fr(rotaryFr)),
        Track(Fr(rotaryFr)),
        Track(Fr(1))
    };
    
    grid.autoRows = Track(Fr(1));
    
    grid.items =
    {
        juce::GridItem(*attackRotary),
        juce::GridItem(*releaseRotary),
        juce::GridItem(*thresholdRotary),
        juce::GridItem(*makeupGainRotary),
        juce::GridItem(*ratioRotary),
        juce::GridItem(resetButton)
    };
    
    grid.performLayout(getLocalBounds());
}
/*
void CompressorBandControl::initRotarySettings(juce::Slider& rotaryControl,
                                               const double& rangeStart,
                                               const double& rangeEnd,
                                               const double& defaultValue)
{
    
}
*/
