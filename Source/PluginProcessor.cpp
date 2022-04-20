/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
void CompressorBand::prepare(juce::dsp::ProcessSpec& spec)
{
    compressor.prepare(spec);
    gain.prepare(spec);
    gain.setRampDurationSeconds(0.05); // 50ms
}

void CompressorBand::updateCompressor()
{
    compressor.setAttack(attack != nullptr ? attack->get() : 50.f);
    compressor.setRelease(release != nullptr ? release->get() : 250.f);
    compressor.setThreshold(threshold != nullptr ? threshold->get() : 0.f);
    compressor.setRatio(ratio != nullptr ? ratio->getCurrentChoiceName().getFloatValue() : 3.f);
    
    compressorConfigured = true;
}

void CompressorBand::updateGain()
{
    gain.setGainDecibels(makeupGain != nullptr ? makeupGain->get() : 0.f);
    
    gainConfigured = true;
}

void CompressorBand::updateBypassState()
{
    shouldBeBypassed = (bypassed != nullptr ? bypassed->get() : false);
}

void CompressorBand::process(juce::AudioBuffer<float>& buffer)
{
    jassert(compressorConfigured);
    jassert(gainConfigured);
    
    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);
    
    context.isBypassed = shouldBeBypassed;
    
    compressor.process(context);
    gain.process(context);
    
    compressorConfigured = false;
    gainConfigured = false;
}

//==============================================================================
PFMProject12AudioProcessor::PFMProject12AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    auto assignFloatParam = [&apvts = this->apvts](auto& target, const auto& name)
    {
        auto param = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(name));
        jassert(param != nullptr);
        target = param;
    };
    
    auto assignChoiceParam = [&apvts = this->apvts](auto& target, const auto& name)
    {
        auto param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(name));
        jassert(param != nullptr);
        target = param;
    };
    
    auto assignBoolParam = [&apvts = this->apvts](auto& target, const auto& name)
    {
        auto param = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(name));
        jassert(param != nullptr);
        target = param;
    };
    
    assignFloatParam(lowBand.attack, "AttackLowBand");
    assignFloatParam(midBand.attack, "AttackMidBand");
    assignFloatParam(highBand.attack, "AttackHighBand");
    
    assignFloatParam(lowBand.release, "ReleaseLowBand");
    assignFloatParam(midBand.release, "ReleaseMidBand");
    assignFloatParam(highBand.release, "ReleaseHighBand");
    
    assignFloatParam(lowBand.threshold, "ThresholdLowBand");
    assignFloatParam(midBand.threshold, "ThresholdMidBand");
    assignFloatParam(highBand.threshold, "ThresholdHighBand");
    
    assignFloatParam(lowBand.makeupGain, "MakeupGainLowBand");
    assignFloatParam(midBand.makeupGain, "MakeupGainMidBand");
    assignFloatParam(highBand.makeupGain, "MakeupGainHighBand");
    
    assignChoiceParam(lowBand.ratio, "RatioLowBand");
    assignChoiceParam(midBand.ratio, "RatioMidBand");
    assignChoiceParam(highBand.ratio, "RatioHighBand");
    
    assignBoolParam(lowBand.bypassed, "BypassedLowBand");
    assignBoolParam(midBand.bypassed, "BypassedMidBand");
    assignBoolParam(highBand.bypassed, "BypassedHighBand");
    
    assignBoolParam(lowBand.solo, "SoloLowBand");
    assignBoolParam(midBand.solo, "SoloMidBand");
    assignBoolParam(highBand.solo, "SoloHighBand");
    
    assignBoolParam(lowBand.mute, "MuteLowBand");
    assignBoolParam(midBand.mute, "MuteMidBand");
    assignBoolParam(highBand.mute, "MuteHighBand");
    
    assignFloatParam(lowMidCrossover, "LowMidCrossover");
    assignFloatParam(midHighCrossover, "MidHighCrossover");
    
    LP1.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    HP1.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    AP2.setType(juce::dsp::LinkwitzRileyFilterType::allpass);
    LP2.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    HP2.setType(juce::dsp::LinkwitzRileyFilterType::highpass);

//    invAP1.setType(juce::dsp::LinkwitzRileyFilterType::allpass);
//    invAP2.setType(juce::dsp::LinkwitzRileyFilterType::allpass);
}

PFMProject12AudioProcessor::~PFMProject12AudioProcessor()
{
}

//==============================================================================
const juce::String PFMProject12AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PFMProject12AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PFMProject12AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PFMProject12AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PFMProject12AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PFMProject12AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PFMProject12AudioProcessor::getCurrentProgram()
{
    return 0;
}

void PFMProject12AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String PFMProject12AudioProcessor::getProgramName (int index)
{
    return {};
}

void PFMProject12AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void PFMProject12AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
        
    for ( auto& comp : compressors )
    {
        comp.prepare(spec);
    }
    
    LP1.prepare(spec);
    HP1.prepare(spec);
    AP2.prepare(spec);
    LP2.prepare(spec);
    HP2.prepare(spec);
    
    for ( auto& fBuffer : filterBuffers )
    {
        fBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    }

//    invAP1.prepare(spec);
//    invAP2.prepare(spec);
//    apBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
}

void PFMProject12AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PFMProject12AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PFMProject12AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    for ( auto& fBuffer : filterBuffers )
    {
        fBuffer = buffer;
    }
    
//    apBuffer = buffer;
    
    updateBands();
    
    /*
    Filter network
    xf0    xf1         xf0    xf1
    LP1 -> AP2    ------\               LP tuned to xf0
    HP1 -> LP2;         /-----\         bandpass between xf0 and xf1
       \-> HP2;               /------   HP tuned to xf1
    */
        
    // process filterBuffers[0] - LP1 --> AP2
    auto fb0Block = juce::dsp::AudioBlock<float>(filterBuffers[0]);
    auto fb0Ctx = juce::dsp::ProcessContextReplacing<float>(fb0Block);
    LP1.process(fb0Ctx);
    AP2.process(fb0Ctx);
    
    // process filterBuffers[1] - HP1 --> LP2
    auto fb1Block = juce::dsp::AudioBlock<float>(filterBuffers[1]);
    auto fb1Ctx = juce::dsp::ProcessContextReplacing<float>(fb1Block);
    HP1.process(fb1Ctx);
    
    // the buffer HP1 has just processed filterBuffers[1] needs to be copied to filterBuffers[2] BEFORE LP2 processes it
    filterBuffers[2] = filterBuffers[1];
    
    // resume processing filterBuffers[1]
    LP2.process(fb1Ctx);
    
    // process filterBuffers[2] - (already processed by HP1) --> HP2
    auto fb2Block = juce::dsp::AudioBlock<float>(filterBuffers[2]);
    auto fb2Ctx = juce::dsp::ProcessContextReplacing<float>(fb2Block);
    HP2.process(fb2Ctx);
    
    for ( auto i = 0; i < compressors.size(); ++i )
    {
        compressors[i].process(filterBuffers[i]);
    }
    
    buffer.clear();
    
    bool bandsAreSoloed = false;
    for ( auto& comp : compressors )
    {
        if ( comp.solo->get() )
        {
            bandsAreSoloed = true;
            break;
        }
    }
    
    if ( bandsAreSoloed )
    {
        for ( auto i = 0; i < compressors.size(); ++i )
        {
            if ( compressors[i].solo->get() )
            {
                addBand(buffer, filterBuffers[i]);
            }
        }
    }
    else
    {
        for ( auto i = 0; i < compressors.size(); ++i )
        {
            if ( !compressors[i].mute->get() )
            {
                addBand(buffer, filterBuffers[i]);
            }
        }
    }
    
    /*
    // test with an inverted allpass filter, it should cancel the phase of the post-filtered signal
     
    auto numChannels = buffer.getNumChannels();
    auto numSamples = buffer.getNumSamples();
    
    if ( bandOne.bypassed->get() )
    {
        auto crossoverFreq0 = lowMidCrossover->get();
        auto crossoverFreq1 = midHighCrossover->get();
     
        invAP1.setCutoffFrequency(crossoverFreq0);
        invAP2.setCutoffFrequency(crossoverFreq1);
        
        auto invApBlock = juce::dsp::AudioBlock<float>(apBuffer);
        auto invApCtx = juce::dsp::ProcessContextReplacing<float>(invApBlock);
        
        invAP1.process(invApCtx);
        invAP2.process(invApCtx);
        
        for ( auto i = 0; i < numChannels; ++i )
        {
            juce::FloatVectorOperations::multiply(apBuffer.getWritePointer(i), -1.f, numSamples);
        }
        
        addBand(buffer, apBuffer);
    }
    */
}

void PFMProject12AudioProcessor::addBand(juce::AudioBuffer<float>& target, const juce::AudioBuffer<float>& source)
{
    for ( int channel = 0; channel < source.getNumChannels(); ++channel )
    {
        target.addFrom(channel, 0, source, channel, 0, source.getNumSamples());
    }
}

void PFMProject12AudioProcessor::updateBands()
{
    for ( auto& comp : compressors )
    {
        comp.updateCompressor();
        comp.updateGain();
        comp.updateBypassState();
    }
    
    auto crossoverFreq0 = lowMidCrossover->get();
    auto crossoverFreq1 = midHighCrossover->get();
    
    LP1.setCutoffFrequency(crossoverFreq0);
    HP1.setCutoffFrequency(crossoverFreq0);
    AP2.setCutoffFrequency(crossoverFreq1);
    LP2.setCutoffFrequency(crossoverFreq1);
    HP2.setCutoffFrequency(crossoverFreq1);
}

//==============================================================================
bool PFMProject12AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PFMProject12AudioProcessor::createEditor()
{
//    return new PFMProject12AudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void PFMProject12AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream mos (destData, true);
    apvts.state.writeToStream(mos);
}

void PFMProject12AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if ( tree.isValid() )
    {
        apvts.replaceState(tree);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout PFMProject12AudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    auto attackRange = juce::NormalisableRange<float>(5.f, 250.f, 1.f, 1.f);
    auto releaseRange = juce::NormalisableRange<float>(5.f, 500.f, 1.f, 1.f);
    auto thresholdRange = juce::NormalisableRange<float>(-60.f, 12.f, 1.f, 1.f);
    auto makeupGainRange = juce::NormalisableRange<float>(0.f, 24.f, 1.f, 1.f);
    
    auto ratioChoices = std::vector<double>{ 1.5, 2, 3, 4, 5, 6, 7, 8, 10, 20, 50, 100 };
    juce::StringArray choicesStringArray;
    for ( auto& choice : ratioChoices )
    {
        choicesStringArray.add( juce::String(choice, 1) );
    }
    
    //==============================================================================
    // Low Band
    layout.add(std::make_unique<juce::AudioParameterFloat>("AttackLowBand", "Attack Low", attackRange, 50.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ReleaseLowBand", "Release Low", releaseRange, 250.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ThresholdLowBand", "Threshold Low", thresholdRange, 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("MakeupGainLowBand", "Makeup Gain Low", makeupGainRange, 0.f));
    layout.add(std::make_unique<juce::AudioParameterChoice>("RatioLowBand", "Ratio Low", choicesStringArray, 2)); // 3:1 ratio set as default
    layout.add(std::make_unique<juce::AudioParameterBool>("BypassedLowBand", "Bypass Low", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("SoloLowBand", "Solo Low", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("MuteLowBand", "Mute Low", false));
    
    //==============================================================================
    // Mid Band
    layout.add(std::make_unique<juce::AudioParameterFloat>("AttackMidBand", "Attack Mid", attackRange, 50.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ReleaseMidBand", "Release Mid", releaseRange, 250.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ThresholdMidBand", "Threshold Mid", thresholdRange, 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("MakeupGainMidBand", "Makeup Gain Mid", makeupGainRange, 0.f));
    layout.add(std::make_unique<juce::AudioParameterChoice>("RatioMidBand", "Ratio Mid", choicesStringArray, 2)); // 3:1 ratio set as default
    layout.add(std::make_unique<juce::AudioParameterBool>("BypassedMidBand", "Bypass Mid", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("SoloMidBand", "Solo Mid", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("MuteMidBand", "Mute Mid", false));
    
    //==============================================================================
    // High Band
    layout.add(std::make_unique<juce::AudioParameterFloat>("AttackHighBand", "Attack High", attackRange, 50.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ReleaseHighBand", "Release High", releaseRange, 250.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("ThresholdHighBand", "Threshold High", thresholdRange, 0.f));
    layout.add(std::make_unique<juce::AudioParameterFloat>("MakeupGainHighBand", "Makeup Gain High", makeupGainRange, 0.f));
    layout.add(std::make_unique<juce::AudioParameterChoice>("RatioHighBand", "Ratio High", choicesStringArray, 2)); // 3:1 ratio set as default
    layout.add(std::make_unique<juce::AudioParameterBool>("BypassedHighBand", "Bypass High", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("SoloHighBand", "Solo High", false));
    layout.add(std::make_unique<juce::AudioParameterBool>("MuteHighBand", "Mute High", false));
    
    //==============================================================================
    layout.add(std::make_unique<juce::AudioParameterFloat>("LowMidCrossover",
                                                           "Low-Mid Crossover",
                                                           juce::NormalisableRange<float>(20.f, 999.f, 1.f, 1.f),
                                                           400.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>("MidHighCrossover",
                                                           "Mid-High Crossover",
                                                           juce::NormalisableRange<float>(1000.f, 20000.f, 1.f, 1.f),
                                                           2000.f));
    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PFMProject12AudioProcessor();
}
