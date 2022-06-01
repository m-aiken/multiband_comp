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
    auto bounds = getLocalBounds();
    auto height = bounds.getHeight();
    auto rotaryWidth = bounds.getWidth() * 0.2;
    
    attackRotary.setBounds     (0,                           0, rotaryWidth, height);
    releaseRotary.setBounds    (attackRotary.getRight(),     0, rotaryWidth, height);
    thresholdRotary.setBounds  (releaseRotary.getRight(),    0, rotaryWidth, height);
    makeupGainRotary.setBounds (thresholdRotary.getRight(),  0, rotaryWidth, height);
    ratioRotary.setBounds      (makeupGainRotary.getRight(), 0, rotaryWidth, height);
}

void CompressorBandControl::initRotarySettings(juce::Slider& rotaryControl,
                                               const double& rangeStart,
                                               const double& rangeEnd,
                                               const double& defaultValue)
{
    
}
