/*
  ==============================================================================

    CompressorSelectionControl.cpp
    Created: 6 Jun 2022 11:43:32pm
    Author:  Matt Aiken

  ==============================================================================
*/

#include "CompressorSelectionControl.h"
#include "../Params.h"
#include "../ColourPalette.h"

//==============================================================================
CompressorSelectionControl::CompressorSelectionControl(juce::AudioProcessorValueTreeState& _apvts, const int& _bandNum)
: apvts(_apvts), bandNum(_bandNum)
{
    const auto& params = Params::getParams();
    
    // Select Button
    selectButton.setButtonText(juce::String(bandNum + 1));
    setColors(selectButton, juce::Colours::skyblue);
    
    selectedBandParam = dynamic_cast<juce::RangedAudioParameter*>(apvts.getParameter(params.at(Params::Names::Selected_Band)));
    jassert( selectedBandParam != nullptr );
    
    selectButton.onClick = [&]()
    {
        selectedBandParam->beginChangeGesture();
        selectedBandParam->setValueNotifyingHost(selectedBandParam->convertTo0to1(bandNum));
        selectedBandParam->endChangeGesture();
    };
    
    auto selectedBandCallback = [this](const auto& selectedBandNum)
    {
        selectButton.setToggleState(selectedBandNum == bandNum, juce::NotificationType::dontSendNotification);
        repaint();
    };
    
    selectParamListener = std::make_unique<ParamListener<float>>(*selectedBandParam, selectedBandCallback);
    
    // Solo, Mute, Bypass Buttons
    auto initSMB_Button = [this](auto& button, auto& attachment, auto& listener, const auto& bandControl, const auto& buttonText, const auto& onColour)
    {
        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(apvts, Params::getBandControlParamName(bandControl, bandNum), button);
        
        auto param = dynamic_cast<juce::RangedAudioParameter*>(apvts.getParameter(Params::getBandControlParamName(bandControl, bandNum)));
        jassert( param != nullptr );
        
        button.onClick = [this, &button](){ updateEnablements(&button); };
        
        listener = std::make_unique<ParamListener<float>>(*param, [this](const auto& parameterValue){ updateButtonStates(); });
        
        button.setButtonText(buttonText);
        setColors(button, onColour);
    };
    
    initSMB_Button(soloButton,
                   soloAttachment,
                   soloParamListener,
                   Params::BandControl::Solo,
                   "S",
                   juce::Colours::lightgreen);
    
    initSMB_Button(muteButton,
                   muteAttachment,
                   muteParamListener,
                   Params::BandControl::Mute,
                   "M",
                   juce::Colours::yellow);
    
    initSMB_Button(bypassButton,
                   bypassAttachment,
                   bypassParamListener,
                   Params::BandControl::Bypass,
                   "X",
                   juce::Colours::red);
    
    addAndMakeVisible(selectButton);
    addAndMakeVisible(soloButton);
    addAndMakeVisible(muteButton);
    addAndMakeVisible(bypassButton);
    
    updateButtonStates();
}

void CompressorSelectionControl::paint(juce::Graphics& g)
{
    if ( selectedBandParam->convertFrom0to1(selectedBandParam->getValue()) == bandNum )
    {
        g.setColour(juce::Colours::skyblue);
        auto bounds = getLocalBounds().toFloat();
        auto padding = 5.f;
        g.drawLine(bounds.getX() + padding,
                   bounds.getBottom() - padding,
                   bounds.getRight() - padding,
                   bounds.getBottom() - padding,
                   padding);
    }
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
    selectButton.setToggleState(shouldBeSelected, juce::NotificationType::dontSendNotification);
}

void CompressorSelectionControl::resetSelectButtonToDefaultColors()
{
    setColors(selectButton, juce::Colours::skyblue);
}

void CompressorSelectionControl::setColors(juce::Component& comp, juce::Colour fillColor)
{
    comp.setColour(juce::TextButton::ColourIds::buttonOnColourId, fillColor);
    comp.setColour(juce::TextButton::ColourIds::buttonColourId, ColourPalette::getColour(ColourPalette::Background));
    
    comp.setColour(juce::TextButton::ColourIds::textColourOnId, ColourPalette::getColour(ColourPalette::Background));
    comp.setColour(juce::TextButton::ColourIds::textColourOffId, ColourPalette::getColour(ColourPalette::Text));
    
    comp.repaint();
}

void CompressorSelectionControl::setColors(juce::Component& target, const juce::Component& source)
{
    target.setColour(juce::TextButton::ColourIds::buttonOnColourId, source.findColour(juce::TextButton::ColourIds::buttonOnColourId));
    target.setColour(juce::TextButton::ColourIds::textColourOnId, source.findColour(juce::TextButton::ColourIds::textColourOnId));
    
    /*
    Scenario: if user has this band select, has s/m/b toggled, then selected another band:
    - the background/text for this select button should stay with the s/m/b colour
    - set the select button "off" background/text colour to the s/m/b "on" colour
    */
    
    target.setColour(juce::TextButton::ColourIds::buttonColourId, source.findColour(juce::TextButton::ColourIds::buttonOnColourId));
    target.setColour(juce::TextButton::ColourIds::textColourOffId, source.findColour(juce::TextButton::ColourIds::textColourOnId));

    target.repaint();
}

void CompressorSelectionControl::updateButtonStates()
{
    std::array<juce::Button*, 3> buttonPtrs{ &soloButton, &muteButton, &bypassButton };
    
    bool selected = false;
    for ( auto i = 0; i < buttonPtrs.size(); ++i )
    {
        // if any button is selected, set the select button to it's colour and stop looking
        if ( buttonPtrs[i]->getToggleState() )
        {
            selected = true;
            setColors(selectButton, *buttonPtrs[i]);
            break;
        }
    }
    
    if ( !selected )
    {
        // if none are selected reset the select button's colour
        resetSelectButtonToDefaultColors();
    }
}

void CompressorSelectionControl::updateEnablements(juce::Button* clickedButton)
{
    if ( callbackBlocker == false )
    {
        juce::ScopedValueSetter<bool> svs(callbackBlocker, true);
        
        std::array<juce::Button*, 3> buttonPtrs{ &soloButton, &muteButton, &bypassButton };
        
        for ( auto i = 0; i < buttonPtrs.size(); ++i )
        {
            
            if ( buttonPtrs[i]->getButtonText() == clickedButton->getButtonText() )
            {
                // toggle the clicked button's state
                buttonPtrs[i]->setToggleState(!buttonPtrs[i]->getToggleState(), juce::NotificationType::sendNotification);
            }
            else if ( buttonPtrs[i]->getToggleState() )
            {
                // if the others are in a clicked/true state - set them false
                buttonPtrs[i]->setToggleState(false, juce::NotificationType::sendNotification);
            }
        }
        
        updateButtonStates();
    }
}
