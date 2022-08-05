/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "dsp/AnalyzerProperties.h"

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
    
    rmsInputLevelDb.store(juce::Decibels::gainToDecibels(computeRMSLevel(buffer), Globals::getNegativeInf()));
    
    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);
    
    context.isBypassed = shouldBeBypassed;
    
    compressor.process(context);
    gain.process(context);
    
    compressorConfigured = false;
    gainConfigured = false;
    
    rmsOutputLevelDb.store(juce::Decibels::gainToDecibels(computeRMSLevel(buffer), Globals::getNegativeInf()));
}

float CompressorBand::getRMSInputLevelDb()
{
    return rmsInputLevelDb.load();
}

float CompressorBand::getRMSOutputLevelDb()
{
    return rmsOutputLevelDb.load();
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
    
    auto assignIntParam = [&apvts = this->apvts](auto& target, const auto& name)
    {
        auto param = dynamic_cast<juce::AudioParameterInt*>(apvts.getParameter(name));
        jassert(param != nullptr);
        target = param;
    };
    
    for ( auto i = 0; i < compressors.size(); ++i )
    {
        assignFloatParam  (compressors[i].attack,     Params::getBandControlParamName(Params::BandControl::Attack, i));
        assignFloatParam  (compressors[i].release,    Params::getBandControlParamName(Params::BandControl::Release, i));
        assignFloatParam  (compressors[i].threshold,  Params::getBandControlParamName(Params::BandControl::Threshold, i));
        assignFloatParam  (compressors[i].makeupGain, Params::getBandControlParamName(Params::BandControl::Gain, i));
        assignChoiceParam (compressors[i].ratio,      Params::getBandControlParamName(Params::BandControl::Ratio, i));
        assignBoolParam   (compressors[i].bypassed,   Params::getBandControlParamName(Params::BandControl::Bypass, i));
        assignBoolParam   (compressors[i].solo,       Params::getBandControlParamName(Params::BandControl::Solo, i));
        assignBoolParam   (compressors[i].mute,       Params::getBandControlParamName(Params::BandControl::Mute, i));
    }
    
    const auto& params = Params::getParams();
    
    assignIntParam(numBands, params.at(Params::Names::Number_Of_Bands));
    assignChoiceParam(processingMode, params.at(Params::Names::Processing_Mode));
    assignFloatParam(gainIn, params.at(Params::Names::Gain_In));
    assignFloatParam(gainOut, params.at(Params::Names::Gain_Out));
    assignIntParam(selectedBand, params.at(Params::Names::Selected_Band));
    
    defaultCenterFrequenciesUpdater = std::make_unique<FifoBackgroundUpdater<int>>([this](const int& nBands){ updateDefaultCenterFrequencies(nBands); });
    
    auto crossoverFreqOrderingUpdaterLambda = [this](const int& nBands)
    {
        auto crossoverParams = getCrossoverParams();
        auto reorderedCrossoverFreqs = getReorderedCrossovers(crossoverParams);
        updateCrossovers(reorderedCrossoverFreqs, crossoverParams);
    };
    
    crossoverFreqOrderingUpdater = std::make_unique<FifoBackgroundUpdater<int>>(crossoverFreqOrderingUpdaterLambda);
    
    const auto& analyzerParams = AnalyzerProperties::getAnalyzerParams();
    
    assignBoolParam(onOffParam, analyzerParams.at(AnalyzerProperties::ParamNames::Enable_Analyzer));
    assignChoiceParam(prePostParam, analyzerParams.at(AnalyzerProperties::ParamNames::Analyzer_Processing_Mode));
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
    
    for ( auto& lmBuffer : leftMidBuffers )
    {
        lmBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock, false, true, true);
        lmBuffer.clear();
    }
    
    for ( auto& rsBuffer : rightSideBuffers )
    {
        rsBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock, false, true, true);
        rsBuffer.clear();
    }
    
    inputGain.prepare(spec);
    outputGain.prepare(spec);
    
    leftSCSF.prepare(samplesPerBlock);
    rightSCSF.prepare(samplesPerBlock);
    
#if USE_TEST_OSC
    testOsc.prepare(spec);
    testOsc.initialise([](float f) { return std::sin(f); });
    
    auto binWidth = sampleRate / 2048;
    auto hz = 2000;
    auto testFrequency = binWidth * (hz / binWidth);
    
    testOsc.setFrequency(testFrequency);
    
    testGain.prepare(spec);
#endif
    
#if TEST_FILTER_NETWORK
    invertedNetwork.resize(MAX_BANDS);
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
    invertedNetwork.updateCutoffs( getDefaultCenterFrequencies(currentNumberOfBands) );
#endif
    
    applyGain(buffer, inputGain);
    
    if (onOffParam->get() && prePostParam->getCurrentChoiceName() == preStr)
    {
        leftSCSF.update(buffer);
        rightSCSF.update(buffer);
    }

    updateMeterFifos(inMeterValuesFifo, buffer);
    
    activeFilterSequence->process(buffer);
    
#if TEST_FILTER_NETWORK
    invertedNetwork.process(buffer);
#endif
    
    auto mode = processingMode->getIndex();
    
    for ( auto i = 0; i < currentNumberOfBands; ++i )
    {
        auto& source = activeFilterSequence->getFilteredBuffer(i);
        const auto& sourceNumSamples = source.getNumSamples();
        
        switch (mode)
        {
            case static_cast<int>(Params::ProcessingMode::Stereo):
            {
                compressors[i].process(source);
                break;
            }
            case static_cast<int>(Params::ProcessingMode::Left):
            case static_cast<int>(Params::ProcessingMode::Right):
            {
                leftMidBuffers[i].clear();
                rightSideBuffers[i].clear();
                
                leftMidBuffers[i].copyFrom(0, 0, source, 0, 0, sourceNumSamples);
                rightSideBuffers[i].copyFrom(1, 0, source, 1, 0, sourceNumSamples);
                
                if ( mode == static_cast<int>(Params::ProcessingMode::Left) )
                    compressors[i].process(leftMidBuffers[i]);
                else
                    compressors[i].process(rightSideBuffers[i]);
                
                break;
            }
            case static_cast<int>(Params::ProcessingMode::Mid):
            case static_cast<int>(Params::ProcessingMode::Side):
            {
                const auto* L = source.getReadPointer(0);
                const auto* R = source.getReadPointer(1);
                auto* M = leftMidBuffers[i].getWritePointer(0);
                auto* S = rightSideBuffers[i].getWritePointer(1);
                
                for ( auto sampleIdx = 0; sampleIdx < sourceNumSamples; ++sampleIdx )
                {
                    M[sampleIdx] = juce::jlimit(-1.f, 1.f, (L[sampleIdx] + R[sampleIdx]) * minusThreeDb);
                    S[sampleIdx] = juce::jlimit(-1.f, 1.f, (L[sampleIdx] - R[sampleIdx]) * minusThreeDb);
                }
                
                if ( mode == static_cast<int>(Params::ProcessingMode::Mid) )
                    compressors[i].process(leftMidBuffers[i]);
                else
                    compressors[i].process(rightSideBuffers[i]);
                
                break;
            }
            default:
                break;
        }
    }
    
    buffer.clear();
    
    const auto& afsBufferCount = activeFilterSequence->getBufferCount();
    const auto& bufferNumSamples = buffer.getNumSamples();
    bool bandsAreSoloed = false;
    for ( auto i = 0; i < afsBufferCount; ++i )
    {
        if ( compressors[i].solo->get() )
        {
            bandsAreSoloed = true;
            break;
        }
    }
    
    if ( bandsAreSoloed )
    {
        for ( auto i = 0; i < afsBufferCount; ++i )
        {
            if ( compressors[i].solo->get() )
            {
                handleProcessingMode(mode, buffer, bufferNumSamples, i);
            }
        }
    }
    else
    {
        for ( auto i = 0; i < afsBufferCount; ++i )
        {
            if ( !compressors[i].mute->get() )
            {
                handleProcessingMode(mode, buffer, bufferNumSamples, i);
            }
        }
    }
    
    applyGain(buffer, outputGain);
    
#if USE_TEST_OSC
    buffer.clear();
    
    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);
    testOsc.process(context);
    testGain.setGainDecibels(JUCE_LIVE_CONSTANT(0));
    testGain.process(context);
#endif
    
    updateMeterFifos(outMeterValuesFifo, buffer);
    
    if (onOffParam->get() && prePostParam->getCurrentChoiceName() == postStr)
    {
        leftSCSF.update(buffer);
        rightSCSF.update(buffer);
    }
    
#if USE_TEST_OSC
    buffer.clear();
#endif
    
#if TEST_FILTER_NETWORK
    if ( apvts.getParameter(Params::getBypassParamName(0))->getValue() > 0.5f )
    {
        invertedNetwork.invert();
        addBand(buffer, invertedNetwork.getProcessedBuffer());
    }
#endif
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
    updateNumberOfBands();
    
    for ( auto& comp : compressors )
    {
        comp.updateCompressor();
        comp.updateGain();
        comp.updateBypassState();
    }
    
    auto afSequence = activeFilterSequence;
    auto crossoverFrequencies = getReorderedCrossovers(getCrossoverParams());
    afSequence->updateFilterCutoffs(crossoverFrequencies);
    
    inputGain.setGainDecibels(gainIn->get());
    outputGain.setGainDecibels(gainOut->get());
}

std::vector<juce::RangedAudioParameter*> PFMProject12AudioProcessor::getCrossoverParams()
{
    std::vector<juce::RangedAudioParameter*> crossoverParams(activeFilterSequence->getBufferCount() - 1);
    auto numCrossoverParams = crossoverParams.size();
    for ( auto i = 0; i < numCrossoverParams; ++i )
    {
        auto param = apvts.getParameter(Params::getCrossoverParamName(i, i+1));
        jassert(param != nullptr);
        crossoverParams[i] = param;
    }
    
    return crossoverParams;
}

std::vector<float> PFMProject12AudioProcessor::getReorderedCrossovers(const std::vector<juce::RangedAudioParameter*>& params)
{
    std::vector<float> crossoverParamsHz(params.size());
    auto numCrossoverParams = params.size();
    for ( auto i = 0; i < numCrossoverParams; ++i )
    {
        crossoverParamsHz[i] = params[i]->convertFrom0to1(params[i]->getValue());
    }
    
    std::sort(crossoverParamsHz.begin(), crossoverParamsHz.end());
    
    return crossoverParamsHz;
}

void PFMProject12AudioProcessor::updateCrossovers(std::vector<float> xovers, const std::vector<juce::RangedAudioParameter*>& params)
{
    JUCE_ASSERT_MESSAGE_THREAD;
    jassert(xovers.size() == params.size());
    
    auto numCrossoverParams = params.size();
    for ( auto i = 0; i < numCrossoverParams; ++i )
    {
        params[i]->beginChangeGesture();
        params[i]->setValueNotifyingHost(params[i]->convertTo0to1(xovers[i]));
        params[i]->endChangeGesture();
    }
}

void PFMProject12AudioProcessor::updateNumberOfBands()
{
    size_t currentSelection = numBands->get();
    if ( currentSelection != currentNumberOfBands )
    {
        filterCreator.requestSequence(currentSelection);
    }
    
    Sequence<float>::Ptr newSequence;
    
    if ( filterCreator.getSequence(newSequence) )
    {
        auto sequenceLength = newSequence->getBufferCount();
        newSequence->prepare(spec);
        releasePool.add(activeFilterSequence);
        defaultCenterFrequenciesUpdater->signalUpdateNeeded(static_cast<int>(sequenceLength));
        crossoverFreqOrderingUpdater->signalUpdateNeeded(static_cast<int>(sequenceLength));
        numFilterBands.store(sequenceLength);
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
    return new PFMProject12AudioProcessorEditor (*this);
//    return new juce::GenericAudioProcessorEditor(*this);
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
    
    const auto& params = Params::getParams();
    
    layout.add(std::make_unique<juce::AudioParameterInt>(params.at(Params::Names::Number_Of_Bands),
                                                         params.at(Params::Names::Number_Of_Bands),
                                                         3,
                                                         8,
                                                         Globals::getNumMaxBands()));
    
    //==============================================================================
    
    auto defaultCenterFreqs = getDefaultCenterFrequencies(Globals::getNumMaxBands());
    
    for ( auto i = 0; i < defaultCenterFreqs.size(); ++i )
    {
        layout.add(std::make_unique<AudioParameterFloatWithResettableDefaultValue>(Params::getCrossoverParamName(i, i+1),
                                                                                   Params::getCrossoverParamName(i, i+1),
                                                                                   juce::NormalisableRange<float>(Globals::getMinFrequency(),
                                                                                                                  Globals::getMaxFrequency(),
                                                                                                                  1.f,
                                                                                                                  1.f),
                                                                                   defaultCenterFreqs[i]));
    }
    
    //==============================================================================
    
    juce::StringArray processingModes;
    const auto& modes = Params::getProcessingModes();
    for ( auto mode : modes )
    {
        processingModes.add(mode.second);
    }
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(params.at(Params::Names::Processing_Mode),
                                                            params.at(Params::Names::Processing_Mode),
                                                            processingModes,
                                                            static_cast<int>(Params::ProcessingMode::Stereo)));
    
    //==============================================================================
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(params.at(Params::Names::Gain_In),
                                                           params.at(Params::Names::Gain_In),
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
                                                           0.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(params.at(Params::Names::Gain_Out),
                                                           params.at(Params::Names::Gain_Out),
                                                           juce::NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f),
                                                           0.f));
    
    //==============================================================================
    
    layout.add(std::make_unique<juce::AudioParameterInt>(params.at(Params::Names::Selected_Band),
                                                         params.at(Params::Names::Selected_Band),
                                                         Globals::getMinBandNum(),
                                                         Globals::getMaxBandNum(),
                                                         0));
    
    //==============================================================================
    
    AnalyzerProperties::addAnalyzerParams(layout);
    
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
    layout.add(std::make_unique<juce::AudioParameterFloat>(Params::getBandControlParamName(Params::BandControl::Attack, bandNum),
                                                           Params::getBandControlParamName(Params::BandControl::Attack, bandNum),
                                                           attackRange,
                                                           50.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(Params::getBandControlParamName(Params::BandControl::Release, bandNum),
                                                           Params::getBandControlParamName(Params::BandControl::Release, bandNum),
                                                           releaseRange,
                                                           250.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(Params::getBandControlParamName(Params::BandControl::Threshold, bandNum),
                                                           Params::getBandControlParamName(Params::BandControl::Threshold, bandNum),
                                                           thresholdRange,
                                                           0.f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(Params::getBandControlParamName(Params::BandControl::Gain, bandNum),
                                                           Params::getBandControlParamName(Params::BandControl::Gain, bandNum),
                                                           makeupGainRange,
                                                           0.f));
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(Params::getBandControlParamName(Params::BandControl::Ratio, bandNum),
                                                            Params::getBandControlParamName(Params::BandControl::Ratio, bandNum),
                                                            choicesStringArray,
                                                            2)); // 3:1 ratio set as default
    
    layout.add(std::make_unique<juce::AudioParameterBool>(Params::getBandControlParamName(Params::BandControl::Bypass, bandNum),
                                                          Params::getBandControlParamName(Params::BandControl::Bypass, bandNum),
                                                          false));
    
    layout.add(std::make_unique<juce::AudioParameterBool>(Params::getBandControlParamName(Params::BandControl::Solo, bandNum),
                                                          Params::getBandControlParamName(Params::BandControl::Solo, bandNum),
                                                          false));
    
    layout.add(std::make_unique<juce::AudioParameterBool>(Params::getBandControlParamName(Params::BandControl::Mute, bandNum),
                                                          Params::getBandControlParamName(Params::BandControl::Mute, bandNum),
                                                          false));
}

std::vector<float> PFMProject12AudioProcessor::getDefaultCenterFrequencies(size_t numBands)
{
    jassert( numBands > 1 );
    std::vector<float> centerFrequencies(numBands - 1);
    auto numFrequencies = centerFrequencies.size();
    for ( auto i = 0; i < numFrequencies; ++i )
    {
        centerFrequencies[i] = std::round( juce::mapToLog10(juce::jmap<float>(i+1, 0, numBands, 0.f, 1.f), Globals::getMinFrequency(), Globals::getMaxFrequency()) );
    }
    
    return centerFrequencies;
}

void PFMProject12AudioProcessor::updateDefaultCenterFrequencies(size_t numBands)
{
    auto defaultFreqs = getDefaultCenterFrequencies(numBands);
    for ( auto i = 0; i < defaultFreqs.size(); ++i )
    {
        auto param = dynamic_cast<AudioParameterFloatWithResettableDefaultValue*>(apvts.getParameter(Params::getCrossoverParamName(i, i+1)));
        jassert(param != nullptr);
        param->setDefaultValue(defaultFreqs[i]);
        param->beginChangeGesture();
        param->setValueNotifyingHost(juce::jmap<float>(defaultFreqs[i], param->range.start, param->range.end, 0.f, 1.f));
        param->endChangeGesture();
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PFMProject12AudioProcessor();
}
