/*
  ==============================================================================

    CompressorSelectionControl.cpp
    Created: 6 Jun 2022 11:43:32pm
    Author:  Matt Aiken

  ==============================================================================
*/

#include "CompressorSelectionControl.h"
#include "../Params.h"

//==============================================================================
CompressorSelectionControl::CompressorSelectionControl(juce::AudioProcessorValueTreeState& _apvts, int _bandNum)
: apvts(_apvts), bandNum(_bandNum)
{
    const auto& params = Params::getParams();
    
    // Select Button
    auto selectedParam = apvts.getParameter(params.at(Params::Names::Selected_Band));
    jassert( selectedParam != nullptr );
    
    selectButton.onClick = [this, &selectedParam]()
    {
        selectedParam->beginChangeGesture();
        selectedParam->setValueNotifyingHost(selectedParam->convertTo0to1(bandNum));
        selectedParam->endChangeGesture();
    };
    
    auto selectedBandListenerLambda = [this](const auto& selectedBandNum)
    {
        if ( selectedBandNum == bandNum )
        {
            selectButton.setToggleState(true, juce::NotificationType::dontSendNotification);
        }
    };
    
    selectParamListener = std::make_unique<ParamListener<float>>(*selectedParam, selectedBandListenerLambda);
    
    selectButton.setButtonText(juce::String(bandNum + 1));
    
    // Solo, Mute, Bypass Buttons
    auto initSoloMuteBypassButton = [this](auto& button, auto& listener, const auto& bandControl, const auto& buttonText)
    {
        auto param = apvts.getParameter(Params::getBandControlParamName(bandControl, bandNum));
        jassert( param != nullptr );
        button.setButtonText(buttonText);
        button.onClick = [this, &button](){ updateEnablements(&button); };
        listener = std::make_unique<ParamListener<float>>(*param, [this](const auto& parameterValue){ updateButtonStates(); });
    };
    
    initSoloMuteBypassButton(soloButton, soloParamListener, Params::BandControl::Solo, "S");
    initSoloMuteBypassButton(muteButton, muteParamListener, Params::BandControl::Mute, "M");
    initSoloMuteBypassButton(bypassButton, bypassParamListener, Params::BandControl::Bypass, "X");
    
    addAndMakeVisible(soloButton);
    addAndMakeVisible(muteButton);
    addAndMakeVisible(bypassButton);
    addAndMakeVisible(selectButton);
    
    updateButtonStates();
}

void CompressorSelectionControl::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::black);
    g.drawRect(getLocalBounds());
}

void CompressorSelectionControl::resized()
{
    auto bounds = getLocalBounds();
    auto soloMuteBypassWidth = bounds.getWidth() * 0.25;
    
    auto selectWidth = soloMuteBypassWidth * 1.4;
    selectButton.setBounds(bounds.getCentreX() - (selectWidth * 0.5),
                           3,
                           selectWidth,
                           selectWidth);
    
    auto soloMuteBypassY = selectButton.getBottom() + 3;
    
    muteButton.setBounds(bounds.getCentreX() - (soloMuteBypassWidth * 0.5),
                         soloMuteBypassY,
                         soloMuteBypassWidth,
                         soloMuteBypassWidth);
    
    soloButton.setBounds(muteButton.getX() - soloMuteBypassWidth - 2,
                         soloMuteBypassY,
                         soloMuteBypassWidth,
                         soloMuteBypassWidth);
    
    bypassButton.setBounds(muteButton.getRight() + 2,
                           soloMuteBypassY,
                           soloMuteBypassWidth,
                           soloMuteBypassWidth);
}

void CompressorSelectionControl::setAsSelected(bool shouldBeSelected)
{
    
}

void CompressorSelectionControl::resetSelectButtonToDefaultColors()
{
    
}

void CompressorSelectionControl::setColors(juce::Component& comp, juce::Colour fillColor, juce::Colour offColor)
{
    
}

void CompressorSelectionControl::setColors(juce::Component& comp, juce::var fillColor, juce::var offColor)
{
    
}

void CompressorSelectionControl::setColors(juce::Component& target, const juce::Component& source)
{
    
}

void CompressorSelectionControl::updateButtonStates()
{
    
}

void CompressorSelectionControl::updateEnablements(juce::Button* clickedButton)
{
    
}
