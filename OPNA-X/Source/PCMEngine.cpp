#include "PCMEngine.h"
#include <BinaryData.h>

PCMEngine::PCMEngine()
{
    formatManager.registerBasicFormats();
}

void PCMEngine::setSampleRate(double newSampleRate)
{
    sampleRate = juce::jmax(1.0, newSampleRate);
    stopAll();
}

void PCMEngine::loadSampleForChannel(int channel, const juce::File& file, bool shouldLoop)
{
    if (!file.existsAsFile())
        return;

    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
    if (reader == nullptr)
        return;

    if (channel < 0 || channel >= static_cast<int>(channels.size()))
        return;

    channels[(size_t)channel].sample.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
    reader->read(&channels[(size_t)channel].sample, 0, (int)reader->lengthInSamples, 0, true, true);
    channels[(size_t)channel].samplePosition = 0;
    channels[(size_t)channel].loop = shouldLoop;
    channels[(size_t)channel].active = false;
}

void PCMEngine::loadSampleFromMemory(int channel, const void* data, size_t sizeInBytes, bool shouldLoop)
{
    if (channel < 0 || channel >= static_cast<int>(channels.size()))
        return;

    auto stream = std::make_unique<juce::MemoryInputStream>(data, sizeInBytes, false);
    std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(std::move(stream)));
    if (reader == nullptr)
        return;

    channels[(size_t)channel].sample.setSize((int)reader->numChannels, (int)reader->lengthInSamples);
    reader->read(&channels[(size_t)channel].sample, 0, (int)reader->lengthInSamples, 0, true, true);
    channels[(size_t)channel].samplePosition = 0;
    channels[(size_t)channel].loop = shouldLoop;
    channels[(size_t)channel].active = false;
}

void PCMEngine::noteOn(int midiNoteNumber)
{
    auto channelIndex = channelForMidi(midiNoteNumber);
    auto& channel = channels[(size_t)channelIndex];
    if (channel.sample.getNumSamples() == 0)
        return;

    channel.samplePosition = 0;
    channel.active = true;
}

void PCMEngine::noteOff(int midiNoteNumber)
{
    auto channelIndex = channelForMidi(midiNoteNumber);
    channels[(size_t)channelIndex].active = false;
}

void PCMEngine::stopAll()
{
    for (auto& channel : channels)
    {
        channel.active = false;
        channel.samplePosition = 0;
    }
}

void PCMEngine::render(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    if (buffer.getNumChannels() == 0)
        return;

    for (auto& channel : channels)
    {
        if (!channel.active || channel.sample.getNumSamples() == 0)
            continue;

        for (int sample = 0; sample < numSamples; ++sample)
        {
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                const int sourceChannel = juce::jmin(ch, channel.sample.getNumChannels() - 1);
                const float value = channel.sample.getSample(sourceChannel, channel.samplePosition);
                buffer.addSample(ch, startSample + sample, value * 0.5f);
            }

            if (++channel.samplePosition >= channel.sample.getNumSamples())
            {
                if (channel.loop)
                    channel.samplePosition = 0;
                else
                {
                    channel.active = false;
                    break;
                }
            }
        }
    }
}

juce::StringArray PCMEngine::getChannelLabels() const
{
    juce::StringArray labels;
    for (size_t i = 0; i < channels.size(); ++i)
        labels.add("PCM " + juce::String((int)i));
    return labels;
}

int PCMEngine::channelForMidi(int midiNoteNumber) const noexcept
{
    return midiNoteNumber % (int)channels.size();
}
