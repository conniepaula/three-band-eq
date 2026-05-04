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
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
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
