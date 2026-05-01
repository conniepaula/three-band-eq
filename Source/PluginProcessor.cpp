/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ThreeBandEQAudioProcessor::ThreeBandEQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

ThreeBandEQAudioProcessor::~ThreeBandEQAudioProcessor()
{
}

//==============================================================================
const juce::String ThreeBandEQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ThreeBandEQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ThreeBandEQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ThreeBandEQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ThreeBandEQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ThreeBandEQAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ThreeBandEQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ThreeBandEQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ThreeBandEQAudioProcessor::getProgramName (int index)
{
    return {};
}

void ThreeBandEQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ThreeBandEQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    juce::dsp::ProcessSpec spec;
    
    spec.maximumBlockSize = samplesPerBlock;
    // we are using two mono chains to process the stereo track
    spec.numChannels = 1;
    spec.sampleRate = sampleRate;
    
    leftChannelChain.prepare(spec);
    rightChannelChain.prepare(spec);
    
    updateFilters();
    };



void ThreeBandEQAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ThreeBandEQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ThreeBandEQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    updateFilters();

    juce::dsp::AudioBlock<float> block(buffer);
    
    auto leftChannelBlock = block.getSingleChannelBlock(0);
    auto rightChannelBlock = block.getSingleChannelBlock(1);
    
    juce::dsp::ProcessContextReplacing<float> leftChannelContext(leftChannelBlock);
    juce::dsp::ProcessContextReplacing<float> rightChannelContext(rightChannelBlock);
    
    leftChannelChain.process(leftChannelContext);
    rightChannelChain.process(rightChannelContext);

}

//==============================================================================
bool ThreeBandEQAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ThreeBandEQAudioProcessor::createEditor()
{
//    return new ThreeBandEQAudioProcessorEditor (*this);
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void ThreeBandEQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::MemoryOutputStream mos(destData, true);
    apvts.state.writeToStream(mos);
}

void ThreeBandEQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid())
    {
        apvts.replaceState(tree);
        updateFilters();
    }
}

ChainSettings getChainSettings(juce::AudioProcessorValueTreeState& apvts)
{
    ChainSettings settings;
    
    settings.lowCutFreq = apvts.getRawParameterValue("lowCutFreq")->load();
    settings.highCutFreq = apvts.getRawParameterValue("highCutFreq")->load();
    settings.peakFreq = apvts.getRawParameterValue("peakFreq")->load();
    settings.peakGainInDecibels = apvts.getRawParameterValue("peakGain")->load();
    settings.peakQuality = apvts.getRawParameterValue("peakQuality")->load();
    settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("lowCutSlope")->load());
    settings.highCutSlope = static_cast<Slope>(apvts.getRawParameterValue("highCutSlope")->load());
    
    return settings;
}

void ThreeBandEQAudioProcessor::updateCoefficients(Coefficients &oldCoefficients, const Coefficients &replacementCoefficients)
{
    // must dereference ref-counted objs allocated on the heap to get underlying object
    *oldCoefficients = *replacementCoefficients;
}

void ThreeBandEQAudioProcessor::updateLowCutFilters(const ChainSettings &chainSettings)
{
    auto lowCutFilterCoefficients = juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(chainSettings.lowCutFreq, getSampleRate(), 2 * (chainSettings.lowCutSlope + 1));
    
    auto& leftLowCutFilter = leftChannelChain.get<ChainPositions::lowCut>();
    auto& rightLowCutFilter = rightChannelChain.get<ChainPositions::lowCut>();
    
    updateCutFilter(leftLowCutFilter, lowCutFilterCoefficients, chainSettings.lowCutSlope);
    updateCutFilter(rightLowCutFilter, lowCutFilterCoefficients, chainSettings.lowCutSlope);
}

void ThreeBandEQAudioProcessor::updateHighCutFilters(const ChainSettings &chainSettings)
{
    auto highCutFilterCoefficients = juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(chainSettings.highCutFreq, getSampleRate(), 2 * (chainSettings.highCutSlope + 1));
    
    auto& leftHighCutFilter = leftChannelChain.get<ChainPositions::highCut>();
    auto& rightHighCutFilter = rightChannelChain.get<ChainPositions::highCut>();
    
    updateCutFilter(leftHighCutFilter, highCutFilterCoefficients, chainSettings.highCutSlope);
    updateCutFilter(rightHighCutFilter, highCutFilterCoefficients, chainSettings.highCutSlope);
}

void ThreeBandEQAudioProcessor::updatePeakFilter(const ChainSettings& chainSettings)
{
    // convert decibels to gain
    float gainFactor = juce::Decibels::decibelsToGain(chainSettings.peakGainInDecibels);
    auto peakCoefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(getSampleRate(),
                                                                        chainSettings.peakFreq,
                                                                        chainSettings.peakQuality,
                                                                        gainFactor);
    
    // ProcessorChain::get<>() selects processor by position number; we use enums for readability's sake
    updateCoefficients(leftChannelChain.get<ChainPositions::peak>().coefficients, peakCoefficients);
    updateCoefficients(rightChannelChain.get<ChainPositions::peak>().coefficients, peakCoefficients);
}

void ThreeBandEQAudioProcessor::updateFilters()
{
    auto chainSettings = getChainSettings(apvts);
    
    updateLowCutFilters(chainSettings);
    updatePeakFilter(chainSettings);
    updateHighCutFilters(chainSettings);
}


juce::AudioProcessorValueTreeState::ParameterLayout ThreeBandEQAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    
    const int version = 1;
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {"lowCutFreq", version},
                                                           "Low Cut Frequency",
                                                           juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f,  0.25f),
                                                           20.0f));
    
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {"highCutFreq", version},
                                                           "High Cut Frequency",
                                                           juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f,  0.25f),
                                                           20000.0f));
    
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {"peakFreq", version},
                                                           "Peak Frequency",
                                                           juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f,  0.25f),
                                                           750.0f));
    
    // TODO: Improve skew factor
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {"peakGain", version},
                                                           "Peak Gain",
                                                           juce::NormalisableRange<float>(-24.0f, 24.0f, 0.5f,  0.25f),
                                                           0.0f));
    // TODO: Improve skew factor
    layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID {"peakQuality", version},
                                                           "Peak Quality",
                                                           juce::NormalisableRange<float>(0.1f, 10.0f, 0.05f,  0.25f),
                                                           1.0f));
    
    juce::StringArray stringArray;
    for (int i = 0; i < 4; ++i)
    {
        juce::String str;
        str << (12 + i*12);
        str << " dB/Oct";
        stringArray.add(str);
    }
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID {"lowCutSlope", version}, "Low Cut Slope", stringArray, 0));
    
    layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID {"highCutSlope", version}, "High Cut Slope", stringArray, 0));

    
    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ThreeBandEQAudioProcessor();
}
