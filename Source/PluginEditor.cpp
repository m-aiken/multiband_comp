/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ColourPalette.h"
#include "Params.h"

//==============================================================================
ControlPlaceholder::ControlPlaceholder(juce::AudioProcessorValueTreeState& _apvts)
: apvts(_apvts)
{
    placeholderButton.setColour(juce::TextButton::ColourIds::buttonColourId, ColourPalette::getColour(ColourPalette::Red));
    placeholderButton.setColour(juce::TextButton::ColourIds::textColourOffId, ColourPalette::getColour(ColourPalette::Text));
    
    resetCompsButton.setColour(juce::TextButton::ColourIds::buttonColourId, ColourPalette::getColour(ColourPalette::Red));
    resetCompsButton.setColour(juce::TextButton::ColourIds::textColourOffId, ColourPalette::getColour(ColourPalette::Text));
    resetCompsButton.onClick = [this](){ resetCompressors(); };
    
    addAndMakeVisible(placeholderButton);
    addAndMakeVisible(resetCompsButton);
}

void ControlPlaceholder::paint(juce::Graphics& g)
{
    g.setColour(ColourPalette::getColour(ColourPalette::Border));
    g.drawRoundedRectangle(getLocalBounds().toFloat(), Globals::getBorderCornerRadius(), Globals::getBorderThickness());
};

void ControlPlaceholder::resized()
{
    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    
    grid.templateColumns = { Track(Fr(1)), Track(Fr(10)), Track(Fr(1)) };
    auto rowPadding = 4;
    grid.templateRows = {
        Track(Fr(rowPadding)),
        Track(Fr(10)),
        Track(Fr(rowPadding)),
        Track(Fr(10)),
        Track(Fr(rowPadding))
    };
    
    grid.items.add(juce::GridItem());
    grid.items.add(juce::GridItem());
    grid.items.add(juce::GridItem());
    grid.items.add(juce::GridItem());
    grid.items.add(juce::GridItem(placeholderButton));
    grid.items.add(juce::GridItem());
    grid.items.add(juce::GridItem());
    grid.items.add(juce::GridItem());
    grid.items.add(juce::GridItem());
    grid.items.add(juce::GridItem());
    grid.items.add(juce::GridItem(resetCompsButton));
    grid.items.add(juce::GridItem());
    grid.items.add(juce::GridItem());
    grid.items.add(juce::GridItem());
    grid.items.add(juce::GridItem());
    
//    grid.setGap(juce::Grid::Px{5});
        
    grid.performLayout(getLocalBounds());
}

void ControlPlaceholder::resetHelper(const juce::String& paramName)
{
    auto param = apvts.getParameter(paramName);
    jassert( param != nullptr );
    
    param->beginChangeGesture();
    param->setValueNotifyingHost(param->getDefaultValue());
    param->endChangeGesture();
}

void ControlPlaceholder::resetCompressors()
{
    for (auto i = 0; i < Globals::getNumMaxBands(); ++i)
    {
        resetHelper(Params::getBandControlParamName(Params::BandControl::Attack,    i));
        resetHelper(Params::getBandControlParamName(Params::BandControl::Release,   i));
        resetHelper(Params::getBandControlParamName(Params::BandControl::Threshold, i));
        resetHelper(Params::getBandControlParamName(Params::BandControl::Gain,      i));
        resetHelper(Params::getBandControlParamName(Params::BandControl::Ratio,     i));
    }
}

//==============================================================================
PFMProject12AudioProcessorEditor::PFMProject12AudioProcessorEditor (PFMProject12AudioProcessor& p)
: AudioProcessorEditor (&p),
  audioProcessor (p),
  spectrumAnalyzer(audioProcessor.getSampleRate(), audioProcessor.leftSCSF, audioProcessor.rightSCSF, audioProcessor.apvts),
  analyzerControls(audioProcessor.apvts),
  modeSelector(audioProcessor.apvts),
  gainInRotary(audioProcessor.apvts, Params::Names::Gain_In),
  gainOutRotary(audioProcessor.apvts, Params::Names::Gain_Out),
  controlPlaceholder(audioProcessor.apvts)
{
    setLookAndFeel(&lnf);
    
    const auto& params = Params::getParams();
    
    bandCountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts,
                                                                                                   params.at(Params::Names::Number_Of_Bands),
                                                                                                   bandCountPicker);
    
    auto numBandsParam = audioProcessor.apvts.getParameter(params.at(Params::Names::Number_Of_Bands));
    jassert(numBandsParam != nullptr);
    
    auto numBandRange = numBandsParam->getNormalisableRange();
    for ( auto i = static_cast<int>(numBandRange.start); i <= static_cast<int>(numBandRange.end); ++i )
    {
        bandCountPicker.addItem(juce::String(i), i); // the range is 3 to 8
    }
    
    auto nBands = numBandsParam->convertFrom0to1(numBandsParam->getValue());
    bandCountPicker.setSelectedId(nBands);
    
    addAndMakeVisible(inStereoMeter);
    addAndMakeVisible(outStereoMeter);
    addAndMakeVisible(bandControls);
    addAndMakeVisible(compSelectionControls);
    addAndMakeVisible(bandCountPicker);
    addAndMakeVisible(spectrumAnalyzer);
    addAndMakeVisible(analyzerControls);
    addAndMakeVisible(modeSelector);
    addAndMakeVisible(gainInRotary);
    addAndMakeVisible(gainOutRotary);
    
    addAndMakeVisible(controlPlaceholder);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(900, 700);
    
    startTimerHz(60);
}

PFMProject12AudioProcessorEditor::~PFMProject12AudioProcessorEditor()
{
    stopTimer();
    setLookAndFeel(nullptr);
}

//==============================================================================
void PFMProject12AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(ColourPalette::getColour(ColourPalette::Background)); // background
}

void PFMProject12AudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto padding = bounds.getWidth() / 50;

    auto stereoMeterWidth = padding * 4;
    
    inStereoMeter.setBounds(padding,                   // x
                            0,                         // y
                            stereoMeterWidth,          // width
                            bounds.getHeight() * 0.8); // height
    
    outStereoMeter.setBounds(bounds.getRight() - stereoMeterWidth - padding,
                             0,
                             stereoMeterWidth,
                             bounds.getHeight() * 0.8);
    
    bandCountPicker.setBounds(bounds.getRight() - 200,
                              padding * 0.5,
                              60,
                              20);
    
    spectrumAnalyzer.setBounds(bounds.getCentreX() - 350, 50, 700, 240);
    
    auto bandControlsHeight = 120;
    auto bandControlsX = inStereoMeter.getRight() + padding;
    auto bandControlsWidth = outStereoMeter.getX() - inStereoMeter.getRight() - (padding * 2);
    auto controlsPadding = padding * 0.5;
    
    compSelectionControls.setBounds(bandControlsX,
                                    spectrumAnalyzer.getBottom() + controlsPadding,
                                    bandControlsWidth,
                                    bandControlsHeight);
    
    bandControls.setBounds(bandControlsX,
                           compSelectionControls.getBottom() + controlsPadding,
                           bandControlsWidth,
                           bandControlsHeight);

    juce::Rectangle<int> bottomControlsBounds;
    bottomControlsBounds.setX(padding);
    bottomControlsBounds.setY(static_cast<int>(std::floor(bandControls.getBottom() + controlsPadding)));
    bottomControlsBounds.setWidth(bounds.getWidth() - (padding * 2));
    bottomControlsBounds.setHeight(bandControlsHeight);
    
    juce::Grid grid;
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    
    grid.templateColumns = { Track(Fr(1)), Track(Fr(1)), Track(Fr(3)), Track(Fr(2)), Track(Fr(1)) };
    grid.autoRows = Track(Fr(1));
    
    grid.items.add(juce::GridItem(gainInRotary));
    grid.items.add(juce::GridItem(modeSelector));
    grid.items.add(juce::GridItem(analyzerControls));
    grid.items.add(juce::GridItem(controlPlaceholder));
    grid.items.add(juce::GridItem(gainOutRotary));
    
    grid.performLayout(bottomControlsBounds);
}

void PFMProject12AudioProcessorEditor::timerCallback()
{
    handleMeterFifo(audioProcessor.inMeterValuesFifo, inMeterValues, inStereoMeter);
    handleMeterFifo(audioProcessor.outMeterValuesFifo, outMeterValues, outStereoMeter);
    
    auto& compressors = audioProcessor.getCompressors();
    std::array<BandLevel, Globals::getNumMaxBands()> levels;
    
    for ( auto i = 0; i < Globals::getNumMaxBands(); ++i )
    {
        BandLevel bandLevel;
        bandLevel.rmsInputLevelDb = compressors[i].getRMSInputLevelDb();
        bandLevel.rmsOutputLevelDb = compressors[i].getRMSOutputLevelDb();
        levels.at(i) = bandLevel;
    }

    compSelectionControls.updateMeters(levels);
    
    auto nFilterBands = audioProcessor.numFilterBands.load();
    if ( nFilterBands != numActiveFilterBands )
    {
        numActiveFilterBands = nFilterBands;
        const auto& params = Params::getParams();
        
        auto selectedBand = audioProcessor.apvts.getParameter(params.at(Params::Names::Selected_Band));
        jassert(selectedBand != nullptr);
        
        if (static_cast<size_t>(selectedBand->convertFrom0to1(selectedBand->getValue())) > numActiveFilterBands - 1)
        {
            selectedBand->beginChangeGesture();
            selectedBand->setValueNotifyingHost(selectedBand->convertTo0to1(numActiveFilterBands - 1));
            selectedBand->endChangeGesture();
        }
        
        compSelectionControls.changeNumBandsDisplayed(static_cast<int>(numActiveFilterBands));
    }
    
    repaint();
}
