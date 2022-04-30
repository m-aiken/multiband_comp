/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

#define MIN_BANDS 0
#define MAX_BANDS 7
#define DISPLAY_FILTER_CONFIGURATIONS true
#define TEST_FILTER_NETWORK true

//==============================================================================
struct InvertedNetwork
{
    void resize(size_t numBands);
    void updateCutoffs(std::vector<float> xoverFreqs);
    const juce::AudioBuffer<float>& getProcessedBuffer();
    void process(const juce::AudioBuffer<float>& inputBuffer);
    void invert();
    void prepare(const juce::dsp::ProcessSpec& spec);
private:
    std::vector<juce::dsp::LinkwitzRileyFilter<float>> allpassFilters;
    juce::AudioBuffer<float> buffer;
    juce::dsp::ProcessSpec currentSpec {44100.0, 512, 2};
        
    void prepareInternal();
};

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
    
    void prepare(juce::dsp::ProcessSpec& spec)
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
        
        prepared = true;
    }
    
    void updateFilterCutoffs(const std::vector<float>& xoverFreqs)
    {
        const juce::ScopedLock scopedFilterLock(filterCS);
        
        if ( xoverFreqs == currentXoverFreqs )
            return;
        
        currentXoverFreqs = xoverFreqs;
        
        jassert( xoverFreqs.size() == mbFilters.size() - 1 );
        
        for ( size_t band = 0; band < mbFilters.size(); ++band )
        {
            auto offset = xoverFreqs.size() - mbFilters[band].size();
            for ( size_t i = 0; i < mbFilters[band].size(); ++i)
            {
                mbFilters[band][i].setCutoffFrequency(xoverFreqs[i+offset]);
            }
        }
        
#if DISPLAY_FILTER_CONFIGURATIONS == true
        juce::String cutoffsTitle("Filter Cutoffs:");
        DBG(cutoffsTitle);
        for ( size_t i = 0; i < mbFilters.size(); ++i )
        {
            juce::String filterBandStr("Band[" + juce::String(i) + "]:");
            
            for ( size_t j = 0; j < mbFilters[i].size(); ++j )
            {
                switch (mbFilters[i][j].getType())
                {
                    case juce::dsp::LinkwitzRileyFilterType::lowpass:
                        filterBandStr += " LP ";
                        break;
                    case juce::dsp::LinkwitzRileyFilterType::highpass:
                        filterBandStr += " HP ";
                        break;
                    case juce::dsp::LinkwitzRileyFilterType::allpass:
                        filterBandStr += " AP ";
                        break;
                    default:
                        break;
                }
                
                filterBandStr += juce::String(mbFilters[i][j].getCutoffFrequency());
            }
            
            DBG(filterBandStr);
        }
#endif
    }
    
    void process(const Buffer& input)
    {
        jassert( prepared );
        const juce::ScopedLock scopedBufferLock(bufferCS);
        
        for ( auto& filterBuffer : filterBuffers )
        {
            filterBuffer.clear();
            filterBuffer = input;
        }
        
        const juce::ScopedLock scopedFilterLock(filterCS);
        
        for ( size_t i = 0; i < filterBuffers.size(); ++i )
        {
            auto block = juce::dsp::AudioBlock<float>(filterBuffers[i]);
            auto context = juce::dsp::ProcessContextReplacing<float>(block);
            
            if ( i == 0 )
            {
                for ( auto& filter : mbFilters[i] )
                {
                    filter.process(context);
                }
            }
            else
            {
                mbFilters[i][0].process(context);
                
                // 1. if final band we will only have index/filter zero ie. mbFilters[i][0]
                // 2. if not final band, we need to copy the buffer that's been processed by filter 0 to the next buffer BEFORE the remaining filters process it
                // 3. then process the current buffer with the remaining filters for this band
                
                if ( i != getBufferCount() - 1 )
                {
                    filterBuffers[i + 1] = filterBuffers[i];
                    
                    for ( size_t j = 1; j < mbFilters[i].size(); ++j )
                    {
                        mbFilters[i][j].process(context);
                    }
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
        
        std::swap(filterBuffers, buffers);
    }
    
    static std::vector<Buffer> createBuffers(size_t numBuffers, int numChannels, int numSamples)
    {
        std::vector<Buffer> buffers;
        for ( size_t i = 0; i < numBuffers; ++i )
        {
            Buffer b;
            b.setSize(numChannels, numSamples, false, true, true);
            buffers.push_back(b);
        }
        return buffers;
    }
                                             
    void createFilters(size_t numBands)
    {
        std::vector<std::vector<Filter>> filterBands;
        for ( size_t i = 0; i < numBands; ++i )
        {
            auto band = createFilterSequence(i, numBands);
            filterBands.push_back(band);
        }
        
#if DISPLAY_FILTER_CONFIGURATIONS == true
        juce::String createdTitle("Created Filters:");
        DBG(createdTitle);
        for ( size_t i = 0; i < filterBands.size(); ++i )
        {
            juce::String filterBandStr("Band[" + juce::String(i) + "]:");
            
            for ( size_t j = 0; j < filterBands[i].size(); ++j )
            {
                switch (filterBands[i][j].getType())
                {
                    case juce::dsp::LinkwitzRileyFilterType::lowpass:
                        filterBandStr += " LP";
                        break;
                    case juce::dsp::LinkwitzRileyFilterType::highpass:
                        filterBandStr += " HP";
                        break;
                    case juce::dsp::LinkwitzRileyFilterType::allpass:
                        filterBandStr += " AP";
                        break;
                    default:
                        break;
                }
            }
            
            DBG(filterBandStr);
        }
#endif
        
        const juce::ScopedLock scopedFilterLock(filterCS);

        std::swap(mbFilters, filterBands);
    }
    
    static std::vector<Filter> createFilterSequence(size_t bandNum, size_t numBands)
    {
        std::vector<Filter> filterSequence;
        
        auto numFiltersNeeded = ( bandNum < 2 ) ? numBands - 1 : numBands - bandNum;
        
        for ( auto i = 0; i < numFiltersNeeded; ++i )
        {
            Filter f;
            filterSequence.push_back(f);
        }
        
        using filterType = juce::dsp::LinkwitzRileyFilterType;
        
        filterSequence[0].setType( ( bandNum == 0 ) ? filterType::lowpass : filterType::highpass );
        
        if ( numFiltersNeeded != 1 )
        {
            for ( auto i = 1; i < numFiltersNeeded; ++i )
            {
                filterSequence[i].setType( ( i == 1 && bandNum != 0 ) ? filterType::lowpass : filterType::allpass );
            }
        }
        
        return filterSequence;
    }
                                                    
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
    static void addBandControls(juce::AudioProcessorValueTreeState::ParameterLayout& layout, const int& bandNum);
    
    juce::AudioProcessorValueTreeState apvts { *this, nullptr, "Parameters", createParameterLayout() };
    
    std::vector<float> createTestCrossovers(const int& numBands);
    
private:
    std::array<CompressorBand, 8> compressors;
    
    FilterSequence<float> filterSequence;
    
    juce::dsp::ProcessSpec spec;
    
    int numBandsLastSelected = 3;
    juce::AudioParameterChoice* numBands { nullptr };
    
#if TEST_FILTER_NETWORK
    InvertedNetwork invertedNetwork;
#endif
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PFMProject12AudioProcessor)
};
