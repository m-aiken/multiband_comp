/*
  ==============================================================================

    CompressorBandControl.cpp
    Created: 1 Jun 2022 12:00:35am
    Author:  Matt Aiken

  ==============================================================================
*/

#include "CompressorBandControl.h"
#include "../Params.h"
#include "../ColourPalette.h"
#include "../Globals.h"

//==============================================================================
CompressorBandControl::CompressorBandControl(juce::AudioProcessorValueTreeState& _apvts)
    : apvts(_apvts)
{
    auto selectedBand = getSelectedBand();
    auto initRotaryControl = [&](auto& rotaryControl, const auto& paramName, const auto& suffix)
    {
        auto param = apvts.getParameter(Params::getBandControlParamName(paramName, selectedBand));
        jassert( param != nullptr );
        
        rotaryControl = std::make_unique<RotaryControl>(*param, suffix, Params::bandControlMap.at(paramName));
    };
    
    initRotaryControl(attackRotary,     Params::BandControl::Attack,    "ms");
    initRotaryControl(releaseRotary,    Params::BandControl::Release,   "ms");
    initRotaryControl(thresholdRotary,  Params::BandControl::Threshold, "dB");
    initRotaryControl(makeupGainRotary, Params::BandControl::Gain,      "dB");
    initRotaryControl(ratioRotary,      Params::BandControl::Ratio,     ":1");
    
    resetButton.setColour(juce::TextButton::ColourIds::buttonColourId, ColourPalette::getColour(ColourPalette::Red));
    resetButton.setColour(juce::TextButton::ColourIds::textColourOffId, ColourPalette::getColour(ColourPalette::Text));
    resetButton.onClick = [this]() { resetParamsToDefault(); };
    
    const auto& params = Params::getParams();
    auto selectedBandParam = dynamic_cast<juce::RangedAudioParameter*>(apvts.getParameter(params.at(Params::Names::Selected_Band)));
    jassert( selectedBandParam != nullptr );
    auto bandSelectCallback = [this](const auto& parameterValue)
    {
        updateRotaryControls();
        updateAttachments();
    };
    
    selectedBandListener = std::make_unique<ParamListener<float>>(*selectedBandParam, bandSelectCallback);
    
    auto msbCallback = [this](const float& parameterValue){ updateEnablements(); };
    for ( auto i = 0; i < Globals::getNumMaxBands(); ++i )
    {
        auto muteParam = dynamic_cast<juce::RangedAudioParameter*>(apvts.getParameter(Params::getBandControlParamName(Params::BandControl::Mute, i)));
        auto soloParam = dynamic_cast<juce::RangedAudioParameter*>(apvts.getParameter(Params::getBandControlParamName(Params::BandControl::Solo, i)));
        auto bypassParam = dynamic_cast<juce::RangedAudioParameter*>(apvts.getParameter(Params::getBandControlParamName(Params::BandControl::Bypass, i)));
        
        jassert( muteParam != nullptr && soloParam != nullptr && bypassParam != nullptr);
        
        muteListeners.push_back(std::make_unique<ParamListener<float>>(*muteParam, msbCallback));
        soloListeners.push_back(std::make_unique<ParamListener<float>>(*soloParam, msbCallback));
        bypassListeners.push_back(std::make_unique<ParamListener<float>>(*bypassParam, msbCallback));
    }
    
    updateAttachments();
    
    addAndMakeVisible(*attackRotary);
    addAndMakeVisible(*releaseRotary);
    addAndMakeVisible(*thresholdRotary);
    addAndMakeVisible(*makeupGainRotary);
    addAndMakeVisible(*ratioRotary);
    
    addAndMakeVisible(resetButton);
    addAndMakeVisible(bandNumButton);
}

void CompressorBandControl::paint(juce::Graphics& g)
{
    g.setColour(ColourPalette::getColour(ColourPalette::Border));
    g.drawRoundedRectangle(getLocalBounds().toFloat(), Globals::getBorderCornerRadius(), Globals::getBorderThickness());
}

void CompressorBandControl::resized()
{
    auto bounds = getLocalBounds();
    auto rotaryBounds = bounds.withWidth(bounds.getWidth() * 0.9);

    // Rotaries
    juce::Grid grid;
    
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    
    grid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)) };
    grid.autoRows = Track(Fr(1));
    
    grid.items =
    {
        juce::GridItem(*attackRotary),
        juce::GridItem(*releaseRotary),
        juce::GridItem(*thresholdRotary),
        juce::GridItem(*makeupGainRotary),
        juce::GridItem(*ratioRotary)
    };
    
    grid.performLayout(rotaryBounds);
    
    // Reset button
    auto resetButtonSize = bounds.getWidth() * 0.06;
    resetButton.setBounds(bounds.getRight() - resetButtonSize - 5,
                          bounds.getCentreY() - (resetButtonSize * 0.5),
                          resetButtonSize,
                          resetButtonSize);
    
    // Label (button)
    auto labelButtonSize = resetButtonSize * 0.5;
    bandNumButton.setBounds(bounds.getRight() - labelButtonSize - 5,
                            5,
                            labelButtonSize,
                            labelButtonSize);
}

void CompressorBandControl::resetHelper(const juce::String& paramName)
{
    auto param = apvts.getParameter(paramName);
    jassert( param != nullptr );
    
    resetHelper(param, param->getDefaultValue());
}

void CompressorBandControl::resetHelper(juce::RangedAudioParameter* param, const float& newValue)
{
    param->beginChangeGesture();
    param->setValueNotifyingHost(newValue);
    param->endChangeGesture();
}

void CompressorBandControl::resetParamsToDefault()
{
    auto selectedBand = getSelectedBand();
    resetHelper(Params::getBandControlParamName(Params::BandControl::Attack,    selectedBand));
    resetHelper(Params::getBandControlParamName(Params::BandControl::Release,   selectedBand));
    resetHelper(Params::getBandControlParamName(Params::BandControl::Threshold, selectedBand));
    resetHelper(Params::getBandControlParamName(Params::BandControl::Gain,      selectedBand));
    resetHelper(Params::getBandControlParamName(Params::BandControl::Ratio,     selectedBand));
}

void CompressorBandControl::updateRotaryControls()
{
    auto selectedBand = getSelectedBand();
    auto updateParamPtr = [&](auto& rotaryControl, const auto& paramName)
    {
        auto param = apvts.getParameter(Params::getBandControlParamName(paramName, selectedBand));
        jassert( param != nullptr );
        
        rotaryControl->updateParam(param);
    };
    
    updateParamPtr(attackRotary,     Params::BandControl::Attack);
    updateParamPtr(releaseRotary,    Params::BandControl::Release);
    updateParamPtr(thresholdRotary,  Params::BandControl::Threshold);
    updateParamPtr(makeupGainRotary, Params::BandControl::Gain);
    updateParamPtr(ratioRotary,      Params::BandControl::Ratio);
}

void CompressorBandControl::updateAttachments()
{
    auto selectedBand = getSelectedBand();
    bandNumButton.setButtonText(juce::String(selectedBand + 1));
    
    attackAttachment.reset();
    releaseAttachment.reset();
    thresholdAttachment.reset();
    makeupGainAttachment.reset();
    ratioAttachment.reset();
    
    attackAttachment     = std::make_unique<Attachment>(apvts, Params::getBandControlParamName(Params::BandControl::Attack,    selectedBand), *attackRotary);
    releaseAttachment    = std::make_unique<Attachment>(apvts, Params::getBandControlParamName(Params::BandControl::Release,   selectedBand), *releaseRotary);
    thresholdAttachment  = std::make_unique<Attachment>(apvts, Params::getBandControlParamName(Params::BandControl::Threshold, selectedBand), *thresholdRotary);
    makeupGainAttachment = std::make_unique<Attachment>(apvts, Params::getBandControlParamName(Params::BandControl::Gain,      selectedBand), *makeupGainRotary);
    ratioAttachment      = std::make_unique<Attachment>(apvts, Params::getBandControlParamName(Params::BandControl::Ratio,     selectedBand), *ratioRotary);
    
    updateEnablements();
}

void CompressorBandControl::updateEnablements()
{
    auto selectedBand = getSelectedBand();
    
    auto muteParam = dynamic_cast<juce::RangedAudioParameter*>(apvts.getParameter(Params::getBandControlParamName(Params::BandControl::Mute, selectedBand)));
    auto bypassParam = dynamic_cast<juce::RangedAudioParameter*>(apvts.getParameter(Params::getBandControlParamName(Params::BandControl::Bypass, selectedBand)));
    jassert( muteParam != nullptr && bypassParam != nullptr);
    
    auto isMutedOrBypassed = ( muteParam->getValue() == 1 || bypassParam->getValue() == 1 );
    
    attackRotary->setEnabled     (!isMutedOrBypassed);
    releaseRotary->setEnabled    (!isMutedOrBypassed);
    thresholdRotary->setEnabled  (!isMutedOrBypassed);
    makeupGainRotary->setEnabled (!isMutedOrBypassed);
    ratioRotary->setEnabled      (!isMutedOrBypassed);
    
    setAlpha(isMutedOrBypassed ? 0.5f : 1.f);
}

int CompressorBandControl::getSelectedBand()
{
    const auto& params = Params::getParams();
    auto selectedBandParam = dynamic_cast<juce::RangedAudioParameter*>(apvts.getParameter(params.at(Params::Names::Selected_Band)));
    jassert( selectedBandParam != nullptr );
        
    return selectedBandParam->convertFrom0to1(selectedBandParam->getValue());
}
