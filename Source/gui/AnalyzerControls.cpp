/*
  ==============================================================================

    AnalyzerControls.cpp
    Created: 29 Jul 2022 12:10:12am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "AnalyzerControls.h"
#include "../dsp/AnalyzerProperties.h"
#include "../ColourPalette.h"
#include "../Globals.h"

//==============================================================================
AnalyzerControls::AnalyzerControls(juce::AudioProcessorValueTreeState& apvts)
{
    const auto& params = AnalyzerProperties::getAnalyzerParams();
    
    auto onOffParam = apvts.getParameter(params.at(AnalyzerProperties::ParamNames::Enable_Analyzer));
    jassert(onOffParam != nullptr);
    onOffButton = std::make_unique<CustomToggleButton>(*onOffParam);
    
    auto prePostParam = apvts.getParameter(params.at(AnalyzerProperties::ParamNames::Analyzer_Processing_Mode));
    jassert(prePostParam != nullptr);
    prePostEqSlider = std::make_unique<CustomLinearSlider>(*prePostParam);
    
    auto fftOrderParam = apvts.getParameter(params.at(AnalyzerProperties::ParamNames::Analyzer_Points));
    jassert(fftOrderParam != nullptr);
    fftOrderSlider = std::make_unique<CustomLinearSlider>(*fftOrderParam);
    
    auto decayRateParam = apvts.getParameter(params.at(AnalyzerProperties::ParamNames::Analyzer_Decay_Rate));
    jassert(decayRateParam);
    decayRateSlider = std::make_unique<RotaryControl>(*decayRateParam, "dB/s", "Decay Rate");
    
    onOffAttachment     = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, params.at(AnalyzerProperties::ParamNames::Enable_Analyzer), *onOffButton);
    prePostEqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, params.at(AnalyzerProperties::ParamNames::Analyzer_Processing_Mode), *prePostEqSlider);
    fftOrderAttachment  = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, params.at(AnalyzerProperties::ParamNames::Analyzer_Points), *fftOrderSlider);
    decayRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, params.at(AnalyzerProperties::ParamNames::Analyzer_Decay_Rate), *decayRateSlider);
    
    onOffButton->onClick = [this]()
    {
        auto toggleState = onOffButton->getToggleState();
        
        prePostEqSlider->setEnabled(toggleState);
        fftOrderSlider->setEnabled(toggleState);
        decayRateSlider->setEnabled(toggleState);
        
        setAlpha(toggleState ? 1.f : 0.5f);
    };
    
    addAndMakeVisible(*onOffButton);
    addAndMakeVisible(*prePostEqSlider);
    addAndMakeVisible(*fftOrderSlider);
    addAndMakeVisible(*decayRateSlider);
}

void AnalyzerControls::paint(juce::Graphics& g)
{
    g.setColour(ColourPalette::getColour(ColourPalette::Border));
    auto bounds = getLocalBounds();
    g.drawRoundedRectangle(bounds.toFloat(), Globals::getBorderCornerRadius(), Globals::getBorderThickness());

    auto quarterWidth = bounds.getWidth() * 0.25;
    auto sliderWidth = 16;
    auto padding = 6;
    auto labelHeight = 20;
    auto textHeight = 12;
    
    g.setColour(ColourPalette::getColour(ColourPalette::Text));
    
    g.drawFittedText("Analyzer",
                     onOffButton->getX(),
                     onOffButton->getY() - labelHeight - padding,
                     onOffButton->getWidth(),
                     labelHeight,
                     juce::Justification::centred, 1);
    
    g.drawFittedText("Input",
                     quarterWidth + sliderWidth,
                     0,
                     quarterWidth,
                     labelHeight,
                     juce::Justification::centredLeft,
                     1);
    
    g.drawFittedText("Points",
                     quarterWidth * 2,
                     0,
                     quarterWidth,
                     labelHeight,
                     juce::Justification::centredLeft,
                     1);
    
    g.setColour(ColourPalette::getColour(ColourPalette::Border));
    g.drawFittedText("Post", prePostEqSlider->getRight() + padding, prePostEqSlider->getY(),                   60, textHeight, juce::Justification::centredLeft, 1);
    g.drawFittedText("Pre",  prePostEqSlider->getRight() + padding, prePostEqSlider->getBottom() - textHeight, 60, textHeight, juce::Justification::centredLeft, 1);
    
    g.drawFittedText("8192", fftOrderSlider->getRight() + padding, fftOrderSlider->getY(),                           60, textHeight, juce::Justification::centredLeft, 1);
    g.drawFittedText("4096", fftOrderSlider->getRight() + padding, (fftOrderSlider->getHeight() * 0.5) + textHeight, 60, textHeight, juce::Justification::centredLeft, 1);
    g.drawFittedText("2048", fftOrderSlider->getRight() + padding, fftOrderSlider->getBottom() - textHeight,         60, textHeight, juce::Justification::centredLeft, 1);
}

void AnalyzerControls::resized()
{
    auto bounds = getLocalBounds();
    auto maxHeight = bounds.getHeight();
    auto quarterWidth = bounds.getWidth() * 0.25;
    auto sliderWidth = 16;
    auto padding = 6;
    auto labelHeight = 20;
    auto buttonHeight = maxHeight * 0.3;
    
    onOffButton->setBounds(padding * 3,
                           bounds.getCentreY() - (buttonHeight * 0.5),
                           quarterWidth - (padding * 5),
                           buttonHeight);
    
    prePostEqSlider->setBounds(quarterWidth + sliderWidth,
                               labelHeight,
                               sliderWidth,
                               maxHeight - labelHeight - padding);
    
    fftOrderSlider->setBounds(quarterWidth * 2,
                              labelHeight,
                              sliderWidth,
                              maxHeight - labelHeight - padding);
    
    decayRateSlider->setBounds(bounds.getRight() - (padding * 2) - quarterWidth,
                               padding,
                               quarterWidth + padding,
                               maxHeight);
}
