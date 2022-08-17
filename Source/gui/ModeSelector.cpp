/*
  ==============================================================================

    ModeSelector.cpp
    Created: 17 Aug 2022 11:47:16pm
    Author:  Matt Aiken

  ==============================================================================
*/

#include "ModeSelector.h"
#include "../Params.h"
#include "../ColourPalette.h"
#include "../Globals.h"

//==============================================================================
ModeSelector::ModeSelector(juce::AudioProcessorValueTreeState& apvts)
{
    const auto params = Params::getParams();
    
    auto modeParam = apvts.getParameter(params.at(Params::Names::Processing_Mode));
    jassert(modeParam != nullptr);
    slider = std::make_unique<CustomLinearSlider>(*modeParam);
    
    sliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, params.at(Params::Names::Processing_Mode), *slider);
    
    addAndMakeVisible(*slider);
}

void ModeSelector::paint(juce::Graphics& g)
{
    g.setColour(ColourPalette::getColour(ColourPalette::Border));
    auto bounds = getLocalBounds();
    g.drawRoundedRectangle(bounds.toFloat(), Globals::getBorderCornerRadius(), Globals::getBorderThickness());
    
    g.setColour(ColourPalette::getColour(ColourPalette::Text));
    
    // Todo - should make a global const for these
    // They're the same values as in AnalyzerControls
    auto labelHeight = 20;
    auto textHeight = 12;
    auto textWidth = 60; // this is hardcoded in AnalyzerControls
    auto padding = 6;
    
    g.drawFittedText("Mode",
                     padding * 2,
                     bounds.getY(),
                     bounds.getWidth(),
                     labelHeight,
                     juce::Justification::centredLeft,
                     1);
    
    auto optionsBounds = juce::Rectangle<int>(slider->getRight() + padding,
                                              slider->getY() - (textHeight * 0.5),
                                              bounds.getWidth() * 0.5,
                                              slider->getHeight() + textHeight);
    
    g.setColour(ColourPalette::getColour(ColourPalette::Border));
    
    juce::StringArray options{"Side", "Mid", "Right", "Left", "Stereo"};
    auto textSegment = optionsBounds.getHeight() * 0.2;
    for (auto i = 0; i < options.size(); ++i)
    {
        g.drawFittedText(options[i],
                         optionsBounds.getX(),
                         optionsBounds.getY() + (textHeight * 0.5) + (textSegment * i),
                         textWidth,
                         textHeight,
                         juce::Justification::centredLeft,
                         1);
    }
}

void ModeSelector::resized()
{
    auto padding = 6;
    auto sliderWidth = 16;
    auto labelHeight = 20;
    
    slider->setBounds(padding * 2,
                      labelHeight,
                      sliderWidth,
                      getLocalBounds().getHeight() - labelHeight - padding);
}
