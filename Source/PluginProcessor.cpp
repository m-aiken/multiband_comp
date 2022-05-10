/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

void InvertedNetwork::resize(size_t numBands)
{
    allpassFilters.clear();
    allpassFilters.assign(numBands - 1, juce::dsp::LinkwitzRileyFilter<float>());
    for ( auto& filter : allpassFilters )
    {
        filter.setType(juce::dsp::LinkwitzRileyFilterType::allpass);
    }
    prepareInternal();
}

void InvertedNetwork::updateCutoffs(std::vector<float> xoverFreqs)
{
    jassert( xoverFreqs.size() == allpassFilters.size() );
    
    for ( size_t i = 0; i < xoverFreqs.size(); ++i )
    {
        allpassFilters[i].setCutoffFrequency(xoverFreqs[i]);
    }
}

const juce::AudioBuffer<float>& InvertedNetwork::getProcessedBuffer() { return buffer; }

void InvertedNetwork::process(const juce::AudioBuffer<float>& inputBuffer)
{
    buffer = inputBuffer;
    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);
    for ( auto& filter : allpassFilters )
    {
        filter.process(context);
    }
}

void InvertedNetwork::invert()
{
    auto numSamples = buffer.getNumSamples();
    for ( auto i = 0; i < buffer.getNumChannels(); ++i )
    {
        juce::FloatVectorOperations::multiply(buffer.getWritePointer(i), -1.f, numSamples);
    }
}

void InvertedNetwork::prepare(const juce::dsp::ProcessSpec& spec)
{
    currentSpec = spec;
    prepareInternal();
}

void InvertedNetwork::prepareInternal()
{
    for ( auto& filter : allpassFilters )
    {
        filter.prepare(currentSpec);
    }
    
    buffer.setSize(currentSpec.numChannels, currentSpec.maximumBlockSize, false, true, true);
    buffer.clear();
}

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
    
    for ( auto i = 0; i < compressors.size(); ++i )
    {
        assignFloatParam  (compressors[i].attack,     Params::getAttackParamName    (i));
        assignFloatParam  (compressors[i].release,    Params::getReleaseParamName   (i));
        assignFloatParam  (compressors[i].threshold,  Params::getThresholdParamName (i));
        assignFloatParam  (compressors[i].makeupGain, Params::getGainParamName      (i));
        assignChoiceParam (compressors[i].ratio,      Params::getRatioParamName     (i));
        assignBoolParam   (compressors[i].bypassed,   Params::getBypassParamName    (i));
        assignBoolParam   (compressors[i].solo,       Params::getSoloParamName      (i));
        assignBoolParam   (compressors[i].mute,       Params::getMuteParamName      (i));
    }
    
    assignChoiceParam(numBands, "NumBands");
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
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
        
    for ( auto& comp : compressors )
    {
        comp.prepare(spec);
    }
    
#if TEST_FILTER_NETWORK
    invertedNetwork.resize(numBands->getCurrentChoiceName().getIntValue());
    invertedNetwork.prepare(spec);
#endif
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

    updateBands();
    
#if TEST_FILTER_NETWORK
    invertedNetwork.resize(currentNumberOfBands);
    invertedNetwork.updateCutoffs(createTestCrossovers(currentNumberOfBands));
#endif

    activeFilterSequence->process(buffer);
    
#if TEST_FILTER_NETWORK
    invertedNetwork.process(buffer);
#endif
    
    for ( auto i = 0; i < currentNumberOfBands; ++i )
    {
        compressors[i].process(activeFilterSequence->getFilteredBuffer(i));
    }
    
    buffer.clear();
    
    bool bandsAreSoloed = false;
    for ( auto i = 0; i < currentNumberOfBands; ++i )
    {
        if ( compressors[i].solo->get() )
        {
            bandsAreSoloed = true;
            break;
        }
    }
    
    if ( bandsAreSoloed )
    {
        for ( auto i = 0; i < currentNumberOfBands; ++i )
        {
            if ( compressors[i].solo->get() )
            {
                addBand(buffer, activeFilterSequence->getFilteredBuffer(i));
            }
        }
    }
    else
    {
        for ( auto i = 0; i < currentNumberOfBands; ++i )
        {
            if ( !compressors[i].mute->get() )
            {
                addBand(buffer, activeFilterSequence->getFilteredBuffer(i));
            }
        }
    }
    
#if TEST_FILTER_NETWORK
    if ( apvts.getParameter(Params::getBypassParamName(0))->getValue() > 0.5f )
    {
        invertedNetwork.invert();
        addBand(buffer, invertedNetwork.getProcessedBuffer());
    }
#endif
}

std::vector<float> PFMProject12AudioProcessor::createTestCrossovers(const size_t& numBands)
{
    std::vector<float> crossovers;
    float interval = 8000.f / numBands;
    float crossover = interval;
    for ( auto i = 0; i < numBands - 1; ++i )
    {
        crossovers.push_back(crossover);
        crossover += interval;
    }
    /*
    juce::String str;
    for ( auto& xOver : crossovers )
    {
        str += " " + juce::String(xOver);
    }
    DBG(str);
    */
    return crossovers;
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
    
    updateNumberOfBands();
    
    auto afSequence = activeFilterSequence;
    
    /*
    std::vector<float> xoverFreqs;
    for ( auto i = 0; i < afSequence->getBufferCount() - 1; ++i )
    {
        auto freq = apvts.getParameter(Params::getCrossoverParamName(i, i+1))->getValue();
        xoverFreqs.push_back(freq);
    }
    */
    auto xoverFreqs = createTestCrossovers(currentNumberOfBands);
    afSequence->updateFilterCutoffs(xoverFreqs);
}

void PFMProject12AudioProcessor::updateNumberOfBands()
{
    size_t currentSelection = numBands->getCurrentChoiceName().getIntValue();
    if ( currentSelection != currentNumberOfBands )
    {
        filterCreator.requestSequence(currentSelection);
    }
    
    Sequence<float>::Ptr newSequence;
    
    if ( filterCreator.getSequence(newSequence) )
    {
        newSequence->prepare(spec);
        releasePool.add(activeFilterSequence);
        activeFilterSequence = newSequence;
        currentNumberOfBands = currentSelection;
    }
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

    addBandControls(layout, 0);
    addBandControls(layout, 1);
    addBandControls(layout, 2);
    addBandControls(layout, 3);
    addBandControls(layout, 4);
    addBandControls(layout, 5);
    addBandControls(layout, 6);
    addBandControls(layout, 7);
    
    layout.add(std::make_unique<juce::AudioParameterChoice>("NumBands",
                                                            "Number Of Bands",
                                                            juce::StringArray{ "2", "3", "4", "5", "6", "7", "8" },
                                                            1)); // 3 set as default
    
    
    //==============================================================================
    layout.add(std::make_unique<juce::AudioParameterFloat>(Params::getCrossoverParamName(0, 1),
                                                           Params::getCrossoverParamName(0, 1),
                                                           juce::NormalisableRange<float>(20.f, 999.f, 1.f, 1.f),
                                                           500.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(Params::getCrossoverParamName(1, 2),
                                                           Params::getCrossoverParamName(1, 2),
                                                           juce::NormalisableRange<float>(1000.f, 1999.f, 1.f, 1.f),
                                                           1500.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(Params::getCrossoverParamName(2, 3),
                                                           Params::getCrossoverParamName(2, 3),
                                                           juce::NormalisableRange<float>(2000.f, 2999.f, 1.f, 1.f),
                                                           2500.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(Params::getCrossoverParamName(3, 4),
                                                           Params::getCrossoverParamName(3, 4),
                                                           juce::NormalisableRange<float>(3000.f, 4999.f, 1.f, 1.f),
                                                           4000.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(Params::getCrossoverParamName(4, 5),
                                                           Params::getCrossoverParamName(4, 5),
                                                           juce::NormalisableRange<float>(5000.f, 7999.f, 1.f, 1.f),
                                                           6500.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(Params::getCrossoverParamName(5, 6),
                                                           Params::getCrossoverParamName(5, 6),
                                                           juce::NormalisableRange<float>(8000.f, 11999.f, 1.f, 1.f),
                                                           10000.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(Params::getCrossoverParamName(6, 7),
                                                           Params::getCrossoverParamName(6, 7),
                                                           juce::NormalisableRange<float>(12000.f, 20000.f, 1.f, 1.f),
                                                           14000.f));
    
    return layout;
}

void PFMProject12AudioProcessor::addBandControls(juce::AudioProcessorValueTreeState::ParameterLayout& layout, const int& bandNum)
{
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
    layout.add(std::make_unique<juce::AudioParameterFloat>(Params::getAttackParamName(bandNum),
                                                           Params::getAttackParamName(bandNum),
                                                           attackRange,
                                                           50.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(Params::getReleaseParamName(bandNum),
                                                           Params::getReleaseParamName(bandNum),
                                                           releaseRange,
                                                           250.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(Params::getThresholdParamName(bandNum),
                                                           Params::getThresholdParamName(bandNum),
                                                           thresholdRange,
                                                           0.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(Params::getGainParamName(bandNum),
                                                           Params::getGainParamName(bandNum),
                                                           makeupGainRange,
                                                           0.f));
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(Params::getRatioParamName(bandNum),
                                                            Params::getRatioParamName(bandNum),
                                                            choicesStringArray,
                                                            2)); // 3:1 ratio set as default
    
    layout.add(std::make_unique<juce::AudioParameterBool>(Params::getBypassParamName(bandNum),
                                                          Params::getBypassParamName(bandNum),
                                                          false));
    
    layout.add(std::make_unique<juce::AudioParameterBool>(Params::getSoloParamName(bandNum),
                                                          Params::getSoloParamName(bandNum),
                                                          false));
    
    layout.add(std::make_unique<juce::AudioParameterBool>(Params::getMuteParamName(bandNum),
                                                          Params::getMuteParamName(bandNum),
                                                          false));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PFMProject12AudioProcessor();
}
