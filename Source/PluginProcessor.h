/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Parameters.h"

enum Slope
{
    slope12,
    slope24,
    slope36,
    slope48
};

struct ChainSettings
{
    float peakFreq { 0 }, peakGainInDecibels { 0 }, peakQuality { 1.f };
    float lowCutFreq { 0 }, highCutFreq { 0 };
    Slope lowCutSlope { slope12 }, highCutSlope { slope12 };
};

ChainSettings getChainSettings(std::vector<param::RAP*>& params);

using Filter = juce::dsp::IIR::Filter<float>;

using CutFilter = juce::dsp::ProcessorChain<Filter, Filter, Filter, Filter>;

using MonoChain = juce::dsp::ProcessorChain<CutFilter, Filter, CutFilter>;

enum ChainPositions
{
    lowCut,
    peak,
    highCut
};


enum CutFilterStage
{
    firstStage,
    secondStage,
    thirdStage,
    fourthStage
};

using Coefficients = Filter::CoefficientsPtr;
void updateCoefficients(Coefficients& oldCoefficients, const Coefficients& replacementCoefficients);

Coefficients makePeakFilter(const ChainSettings& chainSettings, double sampleRate);

template <int Index, typename ChainType, typename CoefficientType>
void updateCutFilterStage(ChainType& chain, const CoefficientType& coefficients)
{
    updateCoefficients(chain.template get<Index>().coefficients, coefficients[Index]);
    chain.template setBypassed<Index>(false);
}

template <typename ChainType, typename CoefficientType>
void updateCutFilter(ChainType& cutFilterChain, const CoefficientType& cutCoefficients, const Slope& cutFilterSlope)
{

    cutFilterChain.template setBypassed<firstStage>(true);
    cutFilterChain.template setBypassed<secondStage>(true);
    cutFilterChain.template setBypassed<thirdStage>(true);
    cutFilterChain.template setBypassed<fourthStage>(true);
    
    switch ( cutFilterSlope )
    {
        case slope48:
        {
            updateCutFilterStage<fourthStage>(cutFilterChain, cutCoefficients);
        }
        case slope36:
        {
            updateCutFilterStage<thirdStage>(cutFilterChain, cutCoefficients);
        }
        case slope24:
        {
            updateCutFilterStage<secondStage>(cutFilterChain, cutCoefficients);
        }
        case slope12:
        {
            updateCutFilterStage<firstStage>(cutFilterChain, cutCoefficients);
        }
    }
}

inline auto makeLowCutFilter(const ChainSettings& chainSettings, double sampleRate)
{
    return juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, sampleRate, 2 * (chainSettings.lowCutSlope + 1));
}

inline auto makeHighCutFilter(const ChainSettings& chainSettings, double sampleRate)
{
    return juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq, sampleRate, 2 * (chainSettings.highCutSlope + 1));
}


//==============================================================================
/**
*/
class ThreeBandEQAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    ThreeBandEQAudioProcessor();
    ~ThreeBandEQAudioProcessor() override;

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

    juce::AudioProcessorValueTreeState apvts;
    std::vector<param::RAP*> params;

private:
    MonoChain leftChannelChain, rightChannelChain;
    
    void updatePeakFilter(const ChainSettings& chainSettings);
    
    void updateLowCutFilters(const ChainSettings& chainSettings);
    void updateHighCutFilters(const ChainSettings& chainSettings);
    
    void updateFilters();
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreeBandEQAudioProcessor)
};
