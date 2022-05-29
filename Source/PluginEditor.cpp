/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DecayingValueHolder::DecayingValueHolder()
{
    setDecayRate(3.f);
    startTimerHz(60);
}

void DecayingValueHolder::updateHeldValue(const float& input)
{
    if ( input > currentValue )
    {
        peakTime = getNow();
        currentValue = input;
        resetDecayRateMultiplier();
    }
}

void DecayingValueHolder::timerCallback()
{
    if ( getNow() - peakTime > holdTime )
    {
        currentValue -= decayRatePerFrame * decayRateMultiplier;
        currentValue = juce::jlimit<float>(NEGATIVE_INFINITY, MAX_DECIBELS, currentValue);
        decayRateMultiplier *= 1.04f;
        
        if ( currentValue == NEGATIVE_INFINITY )
        {
            resetDecayRateMultiplier();
        }
    }
}

//==============================================================================
void DbScale::paint(juce::Graphics& g)
{
    g.drawImage(bkgd, getLocalBounds().toFloat());
}

void DbScale::buildBackgroundImage(int dbDivision, juce::Rectangle<int> meterBounds, int minDb, int maxDb)
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
    auto meterY = meterBounds.getY();
    
    for ( auto i = 0; i < ticks.size(); ++i )
    {
        auto dbString = juce::String(ticks[i].db);
        
        g.drawFittedText((ticks[i].db > 0 ? '+' + dbString : dbString), // text
                         boundsX,                                       // x
                         ticks[i].y + meterY - (textHeight / 2),        // y
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
    
    auto meterHeight = meterBounds.getHeight();
    
    for ( auto db = minDb; db <= maxDb; db += dbDivision )
    {
        Tick tick;
        tick.db = db;
        tick.y = juce::jmap<int>(db, NEGATIVE_INFINITY, MAX_DECIBELS, meterHeight, 0);
        ticks.emplace_back(tick);
    }
    
    return ticks;
}

//==============================================================================
void Meter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    auto componentHeight = bounds.getHeight();
    auto maxMeterHeight = componentHeight * (meterProportion / 100);
    auto meterBoundsYOffset = componentHeight - maxMeterHeight;
    
    g.fillAll(juce::Colours::black); // background
    
    auto maxMeterBounds = juce::Rectangle<int>(0,
                                               meterBoundsYOffset,
                                               bounds.getWidth(),
                                               maxMeterHeight);
    
    // Peak Meter
    g.setColour(juce::Colour(82u, 182u, 154u));
    
    auto peakScaled = juce::jmap<float>(peakDb,
                                        NEGATIVE_INFINITY,
                                        MAX_DECIBELS,
                                        componentHeight,
                                        meterBoundsYOffset);
    
    g.fillRect(maxMeterBounds
                .withHeight(maxMeterHeight * peakScaled)
                .withY(peakScaled));
    
    // Average Meter
    g.setColour(juce::Colour(217u, 237u, 146u));
    
    auto averageScaled = juce::jmap<float>(averageMeter.getAvg(),
                                           NEGATIVE_INFINITY,
                                           MAX_DECIBELS,
                                           componentHeight,
                                           meterBoundsYOffset);
    
    auto avgMeterWidth = bounds.getWidth() * 0.5;
    g.fillRect(maxMeterBounds
                .withHeight(maxMeterHeight * averageScaled)
                .withY(averageScaled)
                .withWidth(avgMeterWidth)
                .withX(bounds.getCentreX() - (avgMeterWidth * 0.5)));
    
    // Falling Tick
    g.setColour( fallingTick.isOverThreshold() ? juce::Colours::red : juce::Colours::white );
    
    auto tickValueScaled = juce::jmap<float>(fallingTick.getCurrentValue(),
                                             NEGATIVE_INFINITY,
                                             MAX_DECIBELS,
                                             componentHeight,
                                             meterBoundsYOffset);
            
    g.drawLine(bounds.getX(),     // startX
               tickValueScaled,   // startY
               bounds.getRight(), // endX
               tickValueScaled,   // endY
               3.f);              // line thickness
    
    
    // L/R label
    g.drawFittedText(labelText,
                     0,
                     0,
                     bounds.getWidth(),
                     componentHeight - maxMeterHeight,
                     juce::Justification::centred,
                     1);
    
    // Meter bounding box/line
    g.setColour(juce::Colours::white);
    g.drawRect(maxMeterBounds);
}

void Meter::update(const float& inputPeakDb, const float& inputRmsDb)
{
    peakDb = inputPeakDb;
    fallingTick.updateHeldValue(inputPeakDb);
    averageMeter.add(inputRmsDb);
    repaint();
}

//==============================================================================
StereoMeter::StereoMeter()
{
    addAndMakeVisible(meterL);
    addAndMakeVisible(meterR);
    addAndMakeVisible(dbScale);
}

void StereoMeter::resized()
{
    auto bounds = getLocalBounds();
    auto padding = bounds.getHeight() / 20;
    auto widthUnit = bounds.getWidth() / 7;
    
    meterL.setBounds(0,                                   // x
                     padding,                             // y
                     widthUnit * 2,                       // width
                     bounds.getHeight() - (padding * 2)); // height
    
    dbScale.setBounds(meterL.getRight(),
                      0,
                      widthUnit * 3,
                      bounds.getHeight());
    
    meterR.setBounds(dbScale.getRight(),
                     padding,
                     widthUnit * 2,
                     bounds.getHeight() - (padding * 2));
    
    /*
    calculate the actual meter bounds so the db scale isn't mapped to a range that includes the label
    */
    auto actualMeterHeight = meterL.getHeight() * (meterL.getMeterProportion() / 100); // to remove the label bounds
    auto meterYOffset = meterL.getHeight() - actualMeterHeight;
    auto meterBoundsForDbScale = juce::Rectangle<int>(meterL.getBounds()).withHeight(actualMeterHeight).withY(meterL.getY() + meterYOffset);
    dbScale.buildBackgroundImage(6, meterBoundsForDbScale, NEGATIVE_INFINITY, MAX_DECIBELS);
}

void StereoMeter::update(const MeterValues& meterValues)
{
    meterL.update(meterValues.leftPeakDb.getDb(), meterValues.leftRmsDb.getDb());
    meterR.update(meterValues.rightPeakDb.getDb(), meterValues.rightRmsDb.getDb());
}

//==============================================================================
PFMProject12AudioProcessorEditor::PFMProject12AudioProcessorEditor (PFMProject12AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    addAndMakeVisible(inStereoMeter);
    addAndMakeVisible(outStereoMeter);
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
    g.fillAll(juce::Colour(105u, 109u, 125u)); // background
}

void PFMProject12AudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto padding = bounds.getWidth() / 40;

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
}

void PFMProject12AudioProcessorEditor::timerCallback()
{
    handleMeterFifo(audioProcessor.inMeterValuesFifo, inMeterValues, inStereoMeter);
    handleMeterFifo(audioProcessor.outMeterValuesFifo, outMeterValues, outStereoMeter);
}

void PFMProject12AudioProcessorEditor::handleMeterFifo(Fifo<MeterValues, 20>& fifo, MeterValues& meterValues, StereoMeter& stereoMeter)
{
    if ( fifo.getNumAvailableForReading() > 0 )
    {
        while ( fifo.pull(meterValues) )
        {
            
        }
        
        stereoMeter.update(meterValues);
    }
}
