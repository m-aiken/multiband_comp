/*
  ==============================================================================

    CompressorSelectionControlContainer.cpp
    Created: 13 Jun 2022 12:22:15am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "CompressorSelectionControlContainer.h"
#include "../Params.h"
#include "../ColourPalette.h"

//==============================================================================
CompressorSelectionControlContainer::CompressorSelectionControlContainer(juce::AudioProcessorValueTreeState& _apvts)
: apvts(_apvts)
{
    changeNumBandsDisplayed(numBandsDisplayed);
}

void CompressorSelectionControlContainer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    g.setColour(ColourPalette::getColour(ColourPalette::Text));
    g.drawRect(bounds);
    auto boundsBottom = bounds.getBottom();
    auto cell = bounds.getWidth() / numBandsDisplayed;
    
    for ( auto i = 1; i < numBandsDisplayed; ++i )
    {
        g.drawLine(cell * i,         // start x
                   1,                // start y
                   cell * i,         // end x
                   boundsBottom - 1, // end y
                   1.f);             // line thickness
    }
}

void CompressorSelectionControlContainer::resized()
{
    juce::Grid grid;
    
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;

    for ( auto& control : controls )
    {
        grid.templateColumns.add( Track(Fr(1)) );
        grid.items.add(juce::GridItem(*control));
    };
    
    grid.autoRows = Track(Fr(1));
    
    grid.performLayout(getLocalBounds());
}

void CompressorSelectionControlContainer::changeNumBandsDisplayed(int numBands)
{
    controls.clear();
    
    const auto& params = Params::getParams();
    auto selectedBandParam = dynamic_cast<juce::RangedAudioParameter*>(apvts.getParameter(params.at(Params::Names::Selected_Band)));
    jassert( selectedBandParam != nullptr );
    auto selectedBandNum = selectedBandParam->getValue();
    
    for ( auto i = 0; i < numBands; ++i )
    {
        controls.push_back(std::make_unique<CompressorSelectionControl>(apvts, i));
        controls[i]->setAsSelected(selectedBandNum == i);
        addAndMakeVisible(*controls[i]);
    }
}
