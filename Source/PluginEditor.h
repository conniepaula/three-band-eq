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
class ThreeBandEQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ThreeBandEQAudioProcessorEditor (ThreeBandEQAudioProcessor&);
    ~ThreeBandEQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ThreeBandEQAudioProcessor& audioProcessor;
    
//    using Slider = juce::Slider;
    using AttachedSliderArray = std::array<AttachedSlider, param::numParams>;
    AttachedSliderArray sliders;
    
    
    AttachedSlider& getSlider(param::PID pID)
    {
        return sliders[static_cast<int>(pID)];
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreeBandEQAudioProcessorEditor)
};
