/*
 ==============================================================================
 
 Parameters.h
 Created: 1 May 2026 2:41:15pm
 Author:  Connie Paula
 
 ==============================================================================
 */

#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace param
{
using APVTS = juce::AudioProcessorValueTreeState;
using Layout = APVTS::ParameterLayout;
using RAP = juce::RangedAudioParameter;
using UniqueRAP = std::unique_ptr<RAP>;
using UniqueRAPVector = std::vector<UniqueRAP>;
using APP = juce::AudioProcessorParameter;
using APF = juce::AudioParameterFloat;
using APC = juce::AudioParameterChoice;
using RangeF = juce::NormalisableRange<float>;

enum class PID {
    lowCutFreq,
    lowCutSlope,
    peakFreq,
    peakGain,
    peakQuality,
    highCutFreq,
    highCutSlope,
    numParams
};

juce::String pidToLabel(PID pID);

juce::String pidToID(PID pID);

juce::String labelToID(const juce::String& label);

juce::String labelToID(PID pID);

static constexpr int numParams = static_cast<int>(PID::numParams);

void createParameter(UniqueRAPVector& vector, PID pID, const RangeF& range, float defaultValue, const juce::String& unit);

void createChoiceParameter(UniqueRAPVector& paramsVector, PID pID, int version, const juce::StringArray& choiceArray, int defaultIndex = 0);

Layout createParameterLayout();

float getParamValue(std::vector<param::RAP*>& params, PID pID);

}
