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
    : apvts(_apvts),
      attackAttachment(apvts, Params::getAttackParamName(0), attackRotary),
      releaseAttachment(apvts, Params::getReleaseParamName(0), releaseRotary),
      thresholdAttachment(apvts, Params::getThresholdParamName(0), thresholdRotary),
      makeupGainAttachment(apvts, Params::getGainParamName(0), makeupGainRotary),
      ratioAttachment(apvts, Params::getRatioParamName(0), ratioRotary)
{
    addAndMakeVisible(attackRotary);
    addAndMakeVisible(releaseRotary);
    addAndMakeVisible(thresholdRotary);
    addAndMakeVisible(makeupGainRotary);
    addAndMakeVisible(ratioRotary);
    
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
        juce::GridItem(attackRotary),
        juce::GridItem(releaseRotary),
        juce::GridItem(thresholdRotary),
        juce::GridItem(makeupGainRotary),
        juce::GridItem(ratioRotary),
        juce::GridItem(resetButton)
    };
    
    grid.performLayout(getLocalBounds());
}

void CompressorBandControl::initRotarySettings(juce::Slider& rotaryControl,
                                               const double& rangeStart,
                                               const double& rangeEnd,
                                               const double& defaultValue)
{
    
}
