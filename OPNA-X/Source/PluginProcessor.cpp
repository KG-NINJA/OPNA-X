#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <BinaryData.h>

OpnaXAudioProcessor::OpnaXAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "Parameters", createParameterLayout())
{
    // PCM demo sample loading (PCM processing entry point).
    for (int channel = 0; channel < 8; ++channel)
        pcmEngine.loadSampleFromMemory(channel, BinaryData::demo_sample_wav, BinaryData::demo_sample_wavSize, true);
}

void OpnaXAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::ignoreUnused(samplesPerBlock);
    fmEngine.setSampleRate(sampleRate);
    pcmEngine.setSampleRate(sampleRate);
    ssgEngine.setSampleRate(sampleRate);

    fmBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    pcmBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    ssgBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
}

void OpnaXAudioProcessor::releaseResources()
{
    fmBuffer.setSize(0, 0);
    pcmBuffer.setSize(0, 0);
    ssgBuffer.setSize(0, 0);
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool OpnaXAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}
#endif

void OpnaXAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    buffer.clear();

    fmBuffer.setSize(numChannels, numSamples, false, false, true);
    pcmBuffer.setSize(numChannels, numSamples, false, false, true);
    ssgBuffer.setSize(numChannels, numSamples, false, false, true);

    fmBuffer.clear();
    pcmBuffer.clear();
    ssgBuffer.clear();

    const auto algorithmValue = static_cast<int>(*parameters.getRawParameterValue("fmAlgorithm"));
    fmEngine.setAlgorithmIndex(algorithmValue);

    const float fmLevel = *parameters.getRawParameterValue("fmLevel");
    const float pcmLevel = *parameters.getRawParameterValue("pcmLevel");
    const float ssgLevel = *parameters.getRawParameterValue("ssgLevel");

    // MIDI dispatch to the three engines (FM/PCM/SSG routing happens here).
    for (const auto metadata : midiMessages)
    {
        const auto& message = metadata.getMessage();
        if (message.isNoteOn())
        {
            const float velocity = message.getFloatVelocity();
            fmEngine.noteOn(message.getNoteNumber(), velocity);
            pcmEngine.noteOn(message.getNoteNumber());
            ssgEngine.noteOn(message.getNoteNumber(), velocity);
        }
        else if (message.isNoteOff())
        {
            fmEngine.noteOff(message.getNoteNumber());
            pcmEngine.noteOff(message.getNoteNumber());
            ssgEngine.noteOff(message.getNoteNumber());
        }
        else if (message.isAllNotesOff() || message.isAllSoundOff())
        {
            fmEngine.allNotesOff();
            pcmEngine.stopAll();
            ssgEngine.allNotesOff();
        }
    }

    fmEngine.render(fmBuffer, 0, numSamples);
    pcmEngine.render(pcmBuffer, 0, numSamples);
    ssgEngine.render(ssgBuffer, 0, numSamples);

    for (int ch = 0; ch < numChannels; ++ch)
    {
        buffer.addFrom(ch, 0, fmBuffer, ch, 0, numSamples, fmLevel);
        buffer.addFrom(ch, 0, pcmBuffer, ch, 0, numSamples, pcmLevel);
        buffer.addFrom(ch, 0, ssgBuffer, ch, 0, numSamples, ssgLevel);
    }
}

bool OpnaXAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* OpnaXAudioProcessor::createEditor()
{
    return new OpnaXAudioProcessorEditor (*this, parameters);
}

void OpnaXAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = parameters.copyState())
    {
        if (auto xml = state.createXml())
            copyXmlToBinary(*xml, destData);
    }
}

void OpnaXAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorValueTreeState::ParameterLayout OpnaXAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    juce::StringArray algorithmChoices;
    for (size_t i = 0; i < fmEngine.getAlgorithms().size(); ++i)
        algorithmChoices.add("Algo " + juce::String((int)i + 1));

    params.push_back(std::make_unique<juce::AudioParameterChoice>("fmAlgorithm", "FM Algorithm", algorithmChoices, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("fmLevel", "FM Level", juce::NormalisableRange<float>(0.0f, 1.0f), 0.7f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("pcmLevel", "PCM Level", juce::NormalisableRange<float>(0.0f, 1.0f), 0.6f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ssgLevel", "SSG Level", juce::NormalisableRange<float>(0.0f, 1.0f), 0.4f));

    return { params.begin(), params.end() };
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OpnaXAudioProcessor();
}
