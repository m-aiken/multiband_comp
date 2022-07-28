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
: AudioProcessorEditor (&p),
  audioProcessor (p),
//  pathProducer(audioProcessor.getSampleRate(), audioProcessor.SCSF),
  spectrumAnalyzer(audioProcessor.getSampleRate(), audioProcessor.leftSCSF, audioProcessor.rightSCSF, audioProcessor.apvts)
{
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
    /*
    pathProducer.changeOrder(FFTOrder::order2048);
    pathProducer.setFFTRectBounds(fftBounds);
    pathProducer.setDecayRate(2.f);
    pathProducer.toggleProcessing(true);
    */
    
    addAndMakeVisible(inStereoMeter);
    addAndMakeVisible(outStereoMeter);
    addAndMakeVisible(bandControls);
    addAndMakeVisible(compSelectionControls);
    addAndMakeVisible(bandCountPicker);
    addAndMakeVisible(spectrumAnalyzer);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(900, 700);
    
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
    g.fillAll(ColourPalette::getColour(ColourPalette::Background)); // background
    
    /*
    // For FFT path test
    auto bounds = getLocalBounds();

    fftBounds.setX(bounds.getCentreX() - (fftBounds.getWidth() * 0.5f));
    fftBounds.setY(50);
    
    g.setColour(ColourPalette::getColour(ColourPalette::Text));
    g.drawRect(fftBounds);
    
    drawFreqLines(g);
    
    fftPath.applyTransform(juce::AffineTransform().translation(fftBounds.getX(), fftBounds.getY()));
    g.setColour(juce::Colours::lightblue);
    g.strokePath(fftPath, juce::PathStrokeType(1.f));
    */
}

void PFMProject12AudioProcessorEditor::drawFreqLines(juce::Graphics& g)
{
    const float minFreq = Globals::getMinFrequency();
    const float maxFreq = Globals::getMaxFrequency();
    auto fftBoundsX = fftBounds.getX();
    auto fftBoundsY = fftBounds.getY();
    auto fftBoundsWidth = fftBounds.getWidth();
    auto fftBoundsBottom = fftBounds.getBottom();
    auto textWidth = 40;
    auto textHeight = 10;

    std::vector<float> freqs { 50.f, 100.f, 500.f, 1000.f, 5000.f, 10000.f, 20000.f };
    
    g.setColour(ColourPalette::getColour(ColourPalette::Text).withLightness(0.3f));
    for ( auto& f : freqs )
    {
        auto normalizedX = juce::mapFromLog10<float>(f, minFreq, maxFreq);
        auto freqLineX = fftBoundsX + fftBoundsWidth * normalizedX;
        g.drawVerticalLine(freqLineX, fftBoundsY, fftBoundsBottom);
        g.drawFittedText(juce::String(f),                          // text
                         freqLineX - (textWidth * 0.5),            // x
                         fftBoundsY - textHeight - 2,                  // y
                         textWidth,                                // width
                         textHeight,                               // height
                         juce::Justification::horizontallyCentred, // justification
                         1);                                       // max num lines
    }
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
                              padding * 0.5,
                              60,
                              20);
    
    auto bandControlsHeight = 120;
    bandControls.setBounds(inStereoMeter.getRight() + padding,
                           inStereoMeter.getBottom() - bandControlsHeight - padding,
                           outStereoMeter.getX() - inStereoMeter.getRight() - (padding * 2),
                           bandControlsHeight);
    
    compSelectionControls.setBounds(bandControls.getX(),
                                    bandControls.getY() - bandControlsHeight - 2,
                                    bandControls.getWidth(),
                                    bandControlsHeight);
    
//    juce::Rectangle<float> fftBounds{600.f, 160.f};
//    auto bounds = getLocalBounds();
//    fftBounds.setX(bounds.getCentreX() - (fftBounds.getWidth() * 0.5f));
//    fftBounds.setY(50);
    spectrumAnalyzer.setBounds(bounds.getCentreX() - 350, 50, 700, 240);
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
    /*
    if ( pathProducer.getNumAvailableForReading() > 0 )
    {
        while ( pathProducer.pull(fftPath) ) { } // get most recent
    }
    */
    repaint();
}
