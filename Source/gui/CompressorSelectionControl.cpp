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
    /*
    auto initSoloMuteBypassButton = [this](const auto& button, const auto& listener, const auto& bandControl, const auto& buttonText)
    {
        auto param = apvts.getParameter(Params::getBandControlParamName(bandControl, bandNum));
        jassert( param != nullptr );
        button.setButtonText(buttonText);
        auto buttonPtr = &button;
        button.onClick = [this](){ updateEnablements(buttonPtr); };
        listener = std::make_unique<ParamListener<float>>(*param, [this](){ updateButtonStates(); });
    };
    
    initSoloMuteBypassButton(soloButton, soloParamListener, Params::BandControl::Solo, "S");
    initSoloMuteBypassButton(muteButton, muteParamListener, Params::BandControl::Mute, "M");
    initSoloMuteBypassButton(bypassButton, bypassParamListener, Params::BandControl::Bypass, "X");
    */
}

void CompressorSelectionControl::resized()
{
    
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
