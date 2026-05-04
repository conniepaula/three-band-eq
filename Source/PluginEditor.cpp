/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Parameters.h"

//==============================================================================
ThreeBandEQAudioProcessorEditor::ThreeBandEQAudioProcessorEditor (ThreeBandEQAudioProcessor& p)
: AudioProcessorEditor (&p),
audioProcessor (p),
sliders{
    AttachedSlider(p, param::PID::lowCutFreq),
    AttachedSlider(p, param::PID::lowCutSlope),
    AttachedSlider(p, param::PID::peakFreq),
    AttachedSlider(p, param::PID::peakGain),
    AttachedSlider(p, param::PID::peakQuality),
    AttachedSlider(p, param::PID::highCutFreq),
    AttachedSlider(p, param::PID::highCutSlope)
}
{
    for (auto& attachedSlider : sliders)
        addAndMakeVisible(attachedSlider.slider);
        
    setSize (600, 400);
}

ThreeBandEQAudioProcessorEditor::~ThreeBandEQAudioProcessorEditor()
{
}

//==============================================================================
void ThreeBandEQAudioProcessorEditor::paint (juce::Graphics& g)
{
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(Colours::black);

    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);
    
    auto w = responseArea.getWidth();
    
    auto& lowCut = monoChain.get<ChainPositions::lowCut>();
    auto& peak = monoChain.get<ChainPositions::peak>();
    auto& highCut = monoChain.get<ChainPositions::highCut>();
    
    auto sampleRate = audioProcessor.getSampleRate();
    
    std::vector<double> magnitudes;
    
    magnitudes.resize(w);
    
    // expressed as gain units -> multiplicative, need a starting gain of 1
    for (int i = 0; i < w; ++i)
    {
        double mag = 1.0f;
        auto freq = mapToLog10(double (i) / double (w), 20.0, 20000.0);
        
        if (monoChain.isBypassed<ChainPositions::peak>())
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        if (lowCut.isBypassed<CutFilterStage::firstStage>())
            mag *= lowCut.get<CutFilterStage::firstStage>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (lowCut.isBypassed<CutFilterStage::secondStage>())
            mag *= lowCut.get<CutFilterStage::secondStage>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (lowCut.isBypassed<CutFilterStage::thirdStage>())
            mag *= lowCut.get<CutFilterStage::thirdStage>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (lowCut.isBypassed<CutFilterStage::fourthStage>())
            mag *= lowCut.get<CutFilterStage::fourthStage>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        if (highCut.isBypassed<CutFilterStage::firstStage>())
            mag *= highCut.get<CutFilterStage::firstStage>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (highCut.isBypassed<CutFilterStage::secondStage>())
            mag *= highCut.get<CutFilterStage::secondStage>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (highCut.isBypassed<CutFilterStage::thirdStage>())
            mag *= highCut.get<CutFilterStage::thirdStage>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        if (highCut.isBypassed<CutFilterStage::fourthStage>())
            mag *= highCut.get<CutFilterStage::fourthStage>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        
        magnitudes[i] = Decibels::gainToDecibels(mag);
    }
    
    Path responseCurve;
    
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input)
    {
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
    };
    
    responseCurve.startNewSubPath(responseArea.getX(), map(magnitudes.front()));
    
    for (size_t i = 1; i< magnitudes.size(); ++i)
    {
        responseCurve.lineTo(responseArea.getX() + i, map(magnitudes[i]));
    }
    
    g.setColour(Colours::orange);
    g.drawRoundedRectangle(responseArea.toFloat(), 4.0f, 1.0f);
    
    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.0f));
}

void ThreeBandEQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);
    
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth() * 0.33);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);

    
    getSlider(param::PID::lowCutFreq).slider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight() * 0.5));
    getSlider(param::PID::lowCutSlope).slider.setBounds(lowCutArea);
    
    getSlider(param::PID::highCutFreq).slider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight() * 0.5));
    getSlider(param::PID::highCutSlope).slider.setBounds(highCutArea);
    

    
    getSlider(param::PID::peakFreq).slider.setBounds(bounds.removeFromTop(bounds.getHeight() * 0.33));
    getSlider(param::PID::peakGain).slider.setBounds(bounds.removeFromTop(bounds.getHeight()  * 0.5f));
    getSlider(param::PID::peakQuality).slider.setBounds(bounds);
}
//}

void ThreeBandEQAudioProcessorEditor::parameterValueChanged(int parameterIndex, float newValue)
{
    parametersChanged.set(true);
}

void ThreeBandEQAudioProcessorEditor::timerCallback()
{
    if (parametersChanged.compareAndSetBool(false, true))
    {
//        if parameters changed, update monochain
//        signal a repaint
    }
}
