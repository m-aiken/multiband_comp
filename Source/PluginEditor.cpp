/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ColourPalette.h"
#include "gui/BandLevel.h"
#include "Params.h"

//==============================================================================
PFMProject12AudioProcessorEditor::PFMProject12AudioProcessorEditor (PFMProject12AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    const auto& params = Params::getParams();
    
    bandCountAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(audioProcessor.apvts,
                                                                                                   params.at(Params::Names::Number_Of_Bands),
                                                                                                   bandCountPicker);
    
    auto numBandRange = audioProcessor.apvts.getParameterRange(params.at(Params::Names::Number_Of_Bands));
    for ( auto i = static_cast<int>(numBandRange.start); i <= static_cast<int>(numBandRange.end); ++i )
    {
        bandCountPicker.addItem(juce::String(i), i); // the range is 2 to 8
    }
    
    addAndMakeVisible(inStereoMeter);
    addAndMakeVisible(outStereoMeter);
    addAndMakeVisible(bandControls);
    addAndMakeVisible(compSelectionControls);
    addAndMakeVisible(bandCountPicker);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(800, 600);
    
    startTimerHz(60);
}

PFMProject12AudioProcessorEditor::~PFMProject12AudioProcessorEditor()
{
    stopTimer();
}

//==============================================================================
void PFMProject12AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
//    g.fillAll(juce::Colour(105u, 109u, 125u)); // background
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
    
#if USE_TEST_OSC
    inStereoMeter.setBounds(padding,
                            JUCE_LIVE_CONSTANT(0),
                            stereoMeterWidth,
                            JUCE_LIVE_CONSTANT(bounds.getHeight() * 0.8));
#endif
    
    bandCountPicker.setBounds(bounds.getRight() - 200,
                              padding,
                              60,
                              40);
    
    auto bandControlsHeight = 120;
    bandControls.setBounds(inStereoMeter.getRight() + padding,
                           inStereoMeter.getBottom() - bandControlsHeight - padding,
                           outStereoMeter.getX() - inStereoMeter.getRight() - (padding * 2),
                           bandControlsHeight);
    
    compSelectionControls.setBounds(bandControls.getX(),
                                    bandControls.getY() - bandControlsHeight - 2,
                                    bandControls.getWidth(),
                                    bandControlsHeight);
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
}
