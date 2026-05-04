#include <JuceHeader.h>
#include "Parameters.h"


namespace param{
juce::String pidToLabel(PID pID)
{
    switch (pID)
    {
        case PID::lowCutFreq:
            return "Low Cut Frequency";
        case PID::lowCutSlope:
            return "Low Cut Slope";
        case PID::peakFreq:
            return "Peak Frequency";
        case PID::peakGain:
            return "Peak Gain";
        case PID::peakQuality:
            return "Peak Quality";
        case PID::highCutFreq:
            return "High Cut Frequency";
        case PID::highCutSlope:
            return "High Cut Slope";
        case PID::numParams:
            break;
    }
    
    jassertfalse;
    return {};
    
}

juce::String labelToID(const juce::String& label)
{
    return label.toLowerCase().removeCharacters(" ");
}

juce::String pidToID(PID pID)
{
    switch (pID)
    {
        case PID::lowCutFreq:
            return "lowCutFreq";
        case PID::lowCutSlope:
            return "lowCutSlope";
        case PID::peakFreq:
            return "peakFreq";
        case PID::peakGain:
            return "peakGain";
        case PID::peakQuality:
            return "peakQuality";
        case PID::highCutFreq:
            return "highCutFreq";
        case PID::highCutSlope:
            return "highCutSlope";
        case PID::numParams:
            break;
    }
    
    jassertfalse;
    return {};
}

void createParameter(UniqueRAPVector& paramsVector, PID pID, const RangeF& range, float defaultValue, const juce::String& unit, const int version)
{
//    const auto valueToString = [](float value, int maxLen)
//    {
//        return juce::String(value, maxLen);
//    };
//    
//    const auto stringToValue = [](const juce::String& text)
//    {
//        return text.getFloatValue();
//    };
//
    const auto id = pidToID(pID);
    const juce::ParameterID jucePID {id, version};
    const auto label = pidToLabel(pID);
    
    
    DBG("Creating parameter: " << label << " | ID: " << id);
    jassert(id.isNotEmpty());
    
    
    paramsVector.push_back(std::make_unique<APF>(jucePID, label, range, defaultValue, unit));
}

void createChoiceParameter(UniqueRAPVector& paramsVector, PID pID, int version, const juce::StringArray& choiceArray, int defaultIndex)
{
    const auto id = pidToID(pID);
    const juce::ParameterID jucePID {id, version};
    const auto label = pidToLabel(pID);
    
    DBG("Creating parameter: " << label << " | ID: " << id);
    jassert(id.isNotEmpty());
    
    paramsVector.push_back(std::make_unique<APC>(jucePID, label, choiceArray, defaultIndex));
}

Layout createParameterLayout()
{
    UniqueRAPVector params;
    const int version = 1;
    juce::StringArray slopes { "12 dB/Oct", "24 dB/Oct", "36 dB/Oct", "48 dB/Oct" };
    
    //    createParameter(params, PID::highCutFreq, RangeF(0.1f, 10.0f, 0.05f,  0.25f), 1.0f, 20000, "Hz");
    createParameter(params, PID::lowCutFreq,
                    RangeF(20.0f, 20000.0f, 1.0f,  0.25f),
                    20.0f,
                    "Hz", version);
    
    createChoiceParameter(params, PID::lowCutSlope, version, slopes);
    

    createParameter(params, PID::peakFreq,
                    RangeF(20.0f, 20000.0f, 1.0f,  0.25f),
                    750.0f,
                    "Hz", version);

    // TODO: Improve skew factor
    createParameter(params, PID::peakGain,
                    RangeF(-24.0f, 24.0f, 0.5f,  0.6f),
                    0.0f,
                    "dB", version);

    // TODO: Improve skew factor
    createParameter(params, PID::peakQuality,
                    RangeF(0.1f, 10.0f, 0.05f,  0.4f),
                    1.0f,
                    "", version);
    

    createParameter(params, PID::highCutFreq, RangeF(20.0f, 20000.0f, 1.0f,  0.25f), 20000.0f, "Hz", version);
    
    
    createChoiceParameter(params, PID::highCutSlope, version, slopes);
    
    return { params.begin(), params.end() };
}

float getParamValue(std::vector<param::RAP*>& params, PID pID)
{
    const auto index = static_cast<int>(pID);
    auto* param = params[index];
    const auto norm = param->getValue();
    return param->getNormalisableRange().convertFrom0to1(norm);

}

}

