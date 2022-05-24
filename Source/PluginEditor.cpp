/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
void DbScale::paint(juce::Graphics& g)
{
    g.drawImage(bkgd, getLocalBounds().toFloat());
}

void DbScale::buildBackgroundImage(int dbDivision, juce::Rectangle<int> meterBounds, int minDb, int maxDb, int yOffset)
{
    jassert( minDb < maxDb );
    
    auto bounds = getLocalBounds();
    if ( bounds.isEmpty() )
        return;
    
    auto scaleFactor = juce::Desktop::getInstance().getGlobalScaleFactor();
    
    bkgd = juce::Image(juce::Image::PixelFormat::RGB,
                       bounds.getWidth() * scaleFactor,
                       bounds.getHeight() * scaleFactor,
                       true);
    
    juce::Graphics g(bkgd);
    g.addTransform(juce::AffineTransform::scale(scaleFactor));
    
    auto ticks = getTicks(dbDivision, meterBounds, minDb, maxDb);
    auto boundsX = bounds.getX();
    auto boundsWidth = bounds.getWidth();
    auto textHeight = 12;
    
    for ( auto i = 0; i < ticks.size(); ++i )
    {
        auto dbString = juce::String(ticks[i].db);
        
        g.drawFittedText((ticks[i].db > 0 ? '+' + dbString : dbString), // text
                         boundsX,                                       // x
                         ticks[i].y + yOffset - (textHeight / 2),       // y
                         boundsWidth,                                   // width
                         textHeight,                                    // height
                         juce::Justification::centred,                  // justification
                         1);                                            // max num lines
    }
}

std::vector<Tick> DbScale::getTicks(int dbDivision, juce::Rectangle<int> meterBounds, int minDb, int maxDb)
{
    if ( minDb > maxDb )
    {
        std::swap(minDb, maxDb);
    }
    
    auto numTicks = (maxDb - minDb) / dbDivision;
    
    std::vector<Tick> ticks;
    ticks.reserve(numTicks);
    
    for ( auto db = minDb; db <= maxDb; db += dbDivision )
    {
        Tick tick;
        tick.db = db;
        tick.y = juce::jmap<int>(db, NEGATIVE_INFINITY, MAX_DECIBELS, meterBounds.getHeight(), 0);
        ticks.emplace_back(tick);
    }
    
    return ticks;
}

//==============================================================================
void Meter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto componentHeight = bounds.getHeight();
    
    g.fillAll(juce::Colour(250u, 243u, 221u)); // background
    
    auto peakScaled = juce::jmap<float>(peakDb, NEGATIVE_INFINITY, MAX_DECIBELS, componentHeight, 0);
    
    g.setColour(juce::Colour(143u, 192u, 169u)); // rectangle colour
    g.fillRect(bounds.withHeight(componentHeight * peakScaled).withY(peakScaled));
}

void Meter::update(float dbLevel)
{
    peakDb = dbLevel;
    repaint();
}

//==============================================================================
PFMProject12AudioProcessorEditor::PFMProject12AudioProcessorEditor (PFMProject12AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible(meter);
    addAndMakeVisible(dbScale);
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize(600, 400);
    
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
    g.fillAll(juce::Colour(105u, 109u, 125u)); // background
}

void PFMProject12AudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto padding = bounds.getWidth() / 20;
    
    meter.setBounds(padding,                             // x
                    padding,                             // y
                    padding,                             // width
                    bounds.getHeight() - (padding * 2)); // height
    
#if USE_TEST_OSC
    meter.setBounds(padding,
                    JUCE_LIVE_CONSTANT(padding),
                    padding,
                    bounds.getHeight() - (padding * 2));
#endif
    
    dbScale.setBounds(meter.getRight(),
                      meter.getY() - (padding / 2),
                      padding,
                      meter.getHeight() + padding);
    
    dbScale.buildBackgroundImage(6, meter.getBounds(), NEGATIVE_INFINITY, MAX_DECIBELS, padding / 2);
}

void PFMProject12AudioProcessorEditor::timerCallback()
{
    if ( audioProcessor.guiFifo.getNumAvailableForReading() > 0 )
    {
        while ( audioProcessor.guiFifo.pull(buffer) )
        {
            // do nothing else - just looping through until incomingBuffer = most recent available buffer
        }
        
        auto leftChannelMag = buffer.getMagnitude(0, 0, buffer.getNumSamples());
        auto leftChannelDb = juce::Decibels::gainToDecibels(leftChannelMag, NEGATIVE_INFINITY);
        meter.update(leftChannelDb);
    }
}
