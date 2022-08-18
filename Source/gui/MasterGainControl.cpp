/*
  ==============================================================================

    MasterGainControl.cpp
    Created: 18 Aug 2022 11:56:42pm
    Author:  Matt Aiken

  ==============================================================================
*/

#include "MasterGainControl.h"
#include "../ColourPalette.h"
#include "../Globals.h"

//==============================================================================
MasterGainControl::MasterGainControl(juce::AudioProcessorValueTreeState& apvts, const Params::Names title)
{
    const auto& params = Params::getParams();
    auto param = apvts.getParameter(params.at(title));
    
    rotary = std::make_unique<RotaryControl>(*param, "dB", params.at(title));
    rotaryAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, params.at(title), *rotary);
    
    addAndMakeVisible(*rotary);
}

void MasterGainControl::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.setColour(ColourPalette::getColour(ColourPalette::Border));
    g.drawRoundedRectangle(bounds.toFloat(), Globals::getBorderCornerRadius(), Globals::getBorderThickness());
}

void MasterGainControl::resized()
{
    auto bounds = getLocalBounds();
    auto padding = 24;
    rotary->setBounds(bounds.withSizeKeepingCentre(bounds.getWidth() - padding,
                                                   bounds.getHeight() - padding));
}
