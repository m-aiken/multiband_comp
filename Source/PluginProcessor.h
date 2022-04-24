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
        
        prepared = true;
    }
    
    void updateFilterCutoffs(const std::vector<float>& xoverFreqs);
    void process(const Buffer& input);
    Buffer& getFilteredBuffer(size_t bandNum);
    size_t getBufferCount() const;
private:
    void createBuffers(size_t numBands);
    static std::vector<Buffer> createBuffers(size_t numBuffers,
                                             int numChannels,
                                             int numSamples);
                                             
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
