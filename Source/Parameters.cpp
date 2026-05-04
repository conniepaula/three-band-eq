#include <JuceHeader.h>
#include "Parameters.h"


namespace param{

juce::String unitToString(Unit unit)
{
    switch (unit) {
        case Unit::decibels:
            return "dB";
        case Unit::hertz:
            return "Hz";
        case Unit::dbOct:
            return "dB/Oct";
        default:
            return "";
    }
}

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

namespace range
{
RangeF biased(float start, float end, float bias) noexcept
{
    // https://www.desmos.com/calculator/ps8q8gftcr
    const auto a = bias * .5f + .5f;
    const auto a2 = 2.f * a;
    const auto aM = 1.f - a;
    
    const auto r = end - start;
    const auto aR = r * a;
    if (bias != 0.f)
        return
    {
        start, end,
        [a2, aM, aR](float min, float, float x)
        {
            const auto denom = aM - x + a2 * x;
            if (denom == 0.f)
                return min;
            return min + aR * x / denom;
        },
        [a2, aM, aR](float min, float, float x)
        {
            const auto denom = a2 * min + aR - a2 * x - min + x;
            if (denom == 0.f)
                return 0.f;
            auto val = aM * (x - min) / denom;
            return val > 1.f ? 1.f : val;
        },
        [](float min, float max, float x)
        {
            return x < min ? min : x > max ? max : x;
        }
    };
    else return { start, end };
}

RangeF stepped(float start, float end, float steps) noexcept
{
    return { start, end, steps };
}

RangeF toggle() noexcept
{
    return stepped(0.f, 1.f);
}

RangeF lin(float start, float end) noexcept
{
    const auto range = end - start;
    
    return
    {
        start,
        end,
        [range](float min, float, float normalized)
        {
            return min + normalized * range;
        },
        [inv = 1.f / range](float min, float, float denormalized)
        {
            return (denormalized - min) * inv;
        },
        [](float min, float max, float x)
        {
            return juce::jlimit(min, max, x);
        }
    };
}

RangeF withCentre(float start, float end, float centre) noexcept
{
    const auto r = end - start;
    const auto v = (centre - start) / r;
    
    return biased(start, end, 2.f * v - 1.f);
}
}

namespace valueToString
{
ValueToString db()
{
    return [](float value, int)
    {
        return juce::String(value, 2) + "dB";
    };
}
ValueToString hz()
{
    return [](float value, int maxLen)
    {
        if (value < 100.0f)
            return juce::String(value, 2) + "Hz";
        else if (value < 1000.0f)
            return juce::String(value, 1) + "Hz";
        else
        {
            auto k = value / 1000.0f;
            return juce::String(k, 1) + "kHz";
        }
    };
}
ValueToString raw()
{
    return [](float value, int)
    {
        return juce::String(value, 2);
    };
}
}

namespace stringToValue
{
StringToValue db()
{
    return [](const juce::String& str)
    {
        return str.removeCharacters(unitToString(Unit::decibels)).getFloatValue();
    };
}
StringToValue hz()
{
    return [](const juce::String& str)
    {
        auto s = str.removeCharacters(unitToString(Unit::hertz));
        if (s.endsWith("k"))
        {
            s = s.dropLastCharacters(1);
            return s.getFloatValue() * 1000.0f;
        }
        return s.getFloatValue();
    };
}
StringToValue raw()
{
    return [](const juce::String& str)
    {
        return str.getFloatValue();
    };
}
}

void createParameter(UniqueRAPVector& paramsVector, PID pID, const RangeF& range, float defaultValue, const Unit& unit, const int version)
{

    ValueToString valToStr;
    StringToValue strToVal;
    
    const auto id = pidToID(pID);
    const juce::ParameterID jucePID {id, version};
    const auto label = pidToLabel(pID);
    
    switch (unit)
    {
        case Unit::decibels:
        {
            valToStr = valueToString::db();
            strToVal = stringToValue::db();
            break;
        }
        case Unit::hertz:
        {
            valToStr = valueToString::hz();
            strToVal = stringToValue::hz();
            break;
        }
        case Unit::unitless:
        {
            valToStr = valueToString::raw();
            strToVal = stringToValue::raw();
            break;
        }
        default:
        {
            jassertfalse;
            break;
        }
    }
    
    
    auto attributes = APFAttributes().withStringFromValueFunction(valToStr).withValueFromStringFunction(strToVal);
    
    
    DBG("Creating parameter: " << label << " | ID: " << id);
    jassert(id.isNotEmpty());
    
    
    paramsVector.push_back(std::make_unique<APF>(jucePID, label, range, defaultValue, attributes));

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
    
    createParameter(params, PID::lowCutFreq,
                    range::withCentre(20.0f, 20000.0f, 1000.0f),
                    20.0f,
                    Unit::hertz, version);
    
    createChoiceParameter(params, PID::lowCutSlope, version, slopes);
    
    
    createParameter(params, PID::peakFreq,
                    range::withCentre(20.0f, 20000.0f, 1000.0f),
                    750.0f,
                    Unit::hertz, version);
    
    // TODO: Improve skew factor
    createParameter(params, PID::peakGain,
                    RangeF(-24.0f, 24.0f, 0.5f,  0.6f),
                    0.0f,
                    Unit::decibels, version);
    
    // TODO: Improve skew factor
    createParameter(params, PID::peakQuality,
                    RangeF(0.1f, 10.0f, 0.05f,  0.4f),
                    1.0f,
                    Unit::unitless, version);
    
    
    createParameter(params, PID::highCutFreq,
                    range::withCentre(20.0f, 20000.0f, 1000.0f),
                    20000.0f,
                    Unit::hertz, version);
    
    
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

