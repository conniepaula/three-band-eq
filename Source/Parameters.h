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
using APFAttributes = juce::AudioParameterFloatAttributes;
using APC = juce::AudioParameterChoice;
using RangeF = juce::NormalisableRange<float>;

namespace range
{
    RangeF biased(float start, float end, float bias) noexcept;
    RangeF stepped(float start, float end, float steps = 1.f) noexcept;
    RangeF toggle() noexcept;
    RangeF lin(float start, float end) noexcept;
    RangeF withCentre(float start, float end, float centre) noexcept;
}

using ValueToString = std::function<juce::String(float, int)>;
using StringToValue = std::function<float(const juce::String&)>;

namespace valueToString
{
ValueToString db();
ValueToString hz();
ValueToString raw();
}

namespace stringToValue
{
StringToValue db();
StringToValue hz();
StringToValue raw();
}

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

enum class Unit {
    decibels,
    hertz,
    unitless,
    dbOct,
    numUnits
};

juce::String unitToString(Unit unit);

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
