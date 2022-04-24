/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define MIN_BANDS 0
#define MAX_BANDS 7

//==============================================================================
template<typename FloatType>
struct FilterSequence
{
    using Filter = juce::dsp::LinkwitzRileyFilter<float>;
    using Buffer = juce::AudioBuffer<float>;
    
    void createBuffersAndFilters(size_t numBands)
    {
        createBuffers(numBands);
        createFilters(numBands);
    }
    
    void prepare(juce::dsp::ProcessSpec spec)
    {
        prepared = false;
        
        numChannels = spec.numChannels;
        numSamples = spec.maximumBlockSize;
        
        for ( auto& band : mbFilters )
        {
            for ( auto& filter : band )
            {
                filter.prepare(spec);
            }
        }
        
        for ( auto& filterBuffer : filterBuffers )
        {
            filterBuffer.setSize(numChannels, numSamples);
        }
        
        prepared = true;
    }
    
    void updateFilterCutoffs(const std::vector<float>& xoverFreqs)
    {
        const juce::ScopedLock scopedFilterLock(filterCS);
        
        if ( xoverFreqs == currentXoverFreqs )
            return;
        
        currentXoverFreqs = xoverFreqs;
        
        jassert( xoverFreqs.size() == mbFilters.size() - 1 );
        
        for ( auto i = 0; i < mbFilters[0].size(); ++i )
        {
            mbFilters[0][i].setCutoffFrequency(xoverFreqs[i]);
        }
        
        for ( auto bandNum = 1; bandNum < mbFilters.size(); ++bandNum )
        {
            auto& band = mbFilters[bandNum];
            
            for ( auto filterIdx = 0; filterIdx < band.size(); ++filterIdx )
            {
                band[filterIdx].setCutoffFrequency(xoverFreqs[filterIdx + bandNum - 1]);
            }
        }
    }
    
    void process(const Buffer& input)
    {
        jassert( prepared );
        const juce::ScopedLock scopedBufferLock(bufferCS);
        
        std::vector<juce::dsp::ProcessContextReplacing<float>> bufferContexts;
        for ( auto& filterBuffer : filterBuffers )
        {
            filterBuffer.clear();
            filterBuffer = input;
            auto block = juce::dsp::AudioBlock<float>(filterBuffer);
            auto context = juce::dsp::ProcessContextReplacing<float>(block);
            bufferContexts.push_back(context);
        }
        
        const juce::ScopedLock scopedFilterLock(filterCS);
        
        // Band 0
        for ( auto& filter : mbFilters[0] )
        {
            filter.process(bufferContexts[0]);
        }
        
        for ( auto band = 1; band < getBufferCount(); ++band )
        {
            mbFilters[band][0].process(bufferContexts[band]);
            
            /*
            1. if final band we will only have index/filter zero ie. mbFilters[band][0]
            2. if not final band, we need to copy the buffer that's been processed by filter 0 to the next buffer BEFORE the remaining filters process it
            3. then process the current buffer with the remaining filters for this band
            */
            
            if ( band != getBufferCount() )
            {
                filterBuffers[band + 1] = filterBuffers[band];
                
                for ( auto filter = 1; filter < mbFilters[band].size(); ++filter )
                {
                    mbFilters[band][filter].process(bufferContexts[band]);
                }
            }
        }
    }
    
    Buffer& getFilteredBuffer(size_t bandNum)
    {
        jassert( bandNum < getBufferCount() );
        return filterBuffers[bandNum];
    }
    
    size_t getBufferCount() const
    {
        return filterBuffers.size();
    }
    
private:
    void createBuffers(size_t numBands)
    {
        auto buffers = createBuffers(numBands, numChannels, numSamples);
        const juce::ScopedLock scopedBufferLock(bufferCS);
        filterBuffers = buffers;
    }
    
    static std::vector<Buffer> createBuffers(size_t numBuffers, int numChannels, int numSamples)
    {
        std::vector<Buffer> buffers;
        for ( auto i = 0; i < numBuffers; ++i )
        {
            Buffer b;
            b.setSize(numChannels, numSamples, false, true, true);
            buffers.push_back(b);
        }
        return buffers;
    }
                                             
    void createFilters(size_t numBands);
    static std::vector<Filter> createFilterSequence(size_t bandNum,
                                                    size_t numBands);
                                                    
    std::vector<std::vector<Filter>> mbFilters;
    std::vector<Buffer> filterBuffers;
    std::vector<float> currentXoverFreqs;
    int numChannels { 2 };
    int numSamples { 512 };
    juce::CriticalSection filterCS, bufferCS;
    bool prepared { false };
};

//==============================================================================
namespace Params
{

inline juce::String getAttackParamName(int bandNum)
{
    juce::String str;
    str << "Band " << bandNum << " Attack";
    return str;
}

inline juce::String getReleaseParamName(int bandNum)
{
    juce::String str;
    str << "Band " << bandNum << " Release";
    return str;
}

inline juce::String getThresholdParamName(int bandNum)
{
    juce::String str;
    str << "Band " << bandNum << " Threshold";
    return str;
}

inline juce::String getGainParamName(int bandNum)
{
    juce::String str;
    str << "Band " << bandNum << " Gain";
    return str;
}

inline juce::String getRatioParamName(int bandNum)
{
    juce::String str;
    str << "Band " << bandNum << " Ratio";
    return str;
}

inline juce::String getBypassParamName(int bandNum)
{
    juce::String str;
    str << "Band " << bandNum << " Bypass";
    return str;
}

inline juce::String getSoloParamName(int bandNum)
{
    juce::String str;
    str << "Band " << bandNum << " Solo";
    return str;
}

inline juce::String getMuteParamName(int bandNum)
{
    juce::String str;
    str << "Band " << bandNum << " Mute";
    return str;
}

inline juce::String getCrossoverParamName(int lowBandNum, int highBandNum)
{
    jassert(lowBandNum < highBandNum);
    jassert(lowBandNum >= MIN_BANDS);
    jassert(highBandNum < MAX_BANDS);
    jassert(highBandNum - lowBandNum == 1);
    
    juce::String str;
    str << "Band " << lowBandNum << "-" << highBandNum << " X-Over";
    return str;
}

}

//==============================================================================
struct CompressorBand
{
    void prepare(juce::dsp::ProcessSpec& spec);
    void updateCompressor();
    void updateGain();
    void updateBypassState();
    void process(juce::AudioBuffer<float>& buffer);
    
    juce::AudioParameterFloat*  attack     { nullptr };
    juce::AudioParameterFloat*  release    { nullptr };
    juce::AudioParameterFloat*  threshold  { nullptr };
    juce::AudioParameterFloat*  makeupGain { nullptr };
    juce::AudioParameterChoice* ratio      { nullptr };
    juce::AudioParameterBool*   bypassed   { nullptr };
    juce::AudioParameterBool*   solo       { nullptr };
    juce::AudioParameterBool*   mute       { nullptr };
    
private:
    bool compressorConfigured = false;
    bool gainConfigured = false;
    bool shouldBeBypassed = false;
    
    juce::dsp::Compressor<float> compressor;
    juce::dsp::Gain<float> gain;
};

//==============================================================================
/**
*/
class PFMProject12AudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PFMProject12AudioProcessor();
    ~PFMProject12AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    void addBand(juce::AudioBuffer<float>& target, const juce::AudioBuffer<float>& source);
    void updateBands();

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Parameters", createParameterLayout() };
    
private:
    std::array<CompressorBand, 3> compressors;
    CompressorBand& lowBand = compressors[0];
    CompressorBand& midBand = compressors[1];
    CompressorBand& highBand = compressors[2];
    
    juce::AudioParameterFloat* lowMidCrossover { nullptr };
    juce::AudioParameterFloat* midHighCrossover { nullptr };
    
    juce::dsp::LinkwitzRileyFilter<float> LP1, AP2,
                                          HP1, LP2,
                                               HP2;
    
    std::array<juce::AudioBuffer<float>, 3> filterBuffers;
    
//    juce::dsp::LinkwitzRileyFilter<float> invAP1, invAP2;
//    juce::AudioBuffer<float> apBuffer;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PFMProject12AudioProcessor)
};
