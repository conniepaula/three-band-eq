/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Parameters.h"

struct AttachedSlider
{
    using Slider = juce::Slider;
    using Attachment = juce::SliderParameterAttachment;
    
    AttachedSlider(ThreeBandEQAudioProcessor& audioProcessor, param::PID pID) : slider(), attachment(*audioProcessor.params[static_cast<int>(pID)], slider, nullptr)
    {
        slider.setSliderStyle(Slider::SliderStyle::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, false, 0, 0);
     
        attachment.sendInitialUpdate();
    }
    
    void addAndMakeVisible(juce::Component& comp)
    {
        comp.addAndMakeVisible(slider);
    }
    
    Slider slider;
    Attachment attachment;
};

struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                        juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
        
    }
};

//==============================================================================
/**
*/
class ThreeBandEQAudioProcessorEditor  : public juce::AudioProcessorEditor, juce::AudioProcessorParameter::Listener, juce::Timer
{
public:
    ThreeBandEQAudioProcessorEditor (ThreeBandEQAudioProcessor&);
    ~ThreeBandEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    void parameterValueChanged (int parameterIndex, float newValue) override;
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override {}
    
    void timerCallback() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ThreeBandEQAudioProcessor& audioProcessor;
    
    juce::Atomic<bool> parametersChanged { false };
    
    using AttachedSliderArray = std::array<AttachedSlider, param::numParams>;
    AttachedSliderArray sliders;
    
    
    AttachedSlider& getSlider(param::PID pID)
    {
        return sliders[static_cast<int>(pID)];
    }
    
    MonoChain monoChain;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreeBandEQAudioProcessorEditor)
};
