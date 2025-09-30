#pragma once

#include <array>
#include <vector>
#include <memory>
#include <juce_audio_formats/juce_audio_formats.h>

// PCMEngine: 8-channel PCM playback with looping support and embedded sample loading.
class PCMEngine
{
public:
    PCMEngine();

    void setSampleRate(double newSampleRate);

    void loadSampleForChannel(int channel, const juce::File& file, bool shouldLoop);
    void loadSampleFromMemory(int channel, const void* data, size_t sizeInBytes, bool shouldLoop);

    void noteOn(int midiNoteNumber);
    void noteOff(int midiNoteNumber);
    void stopAll();

    void render(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    juce::StringArray getChannelLabels() const;

private:
    struct Channel
    {
        juce::AudioBuffer<float> sample;
        int samplePosition = 0;
        bool loop = false;
        bool active = false;
    };

    double sampleRate = 44100.0;
    juce::AudioFormatManager formatManager;
    std::array<Channel, 8> channels{};

    int channelForMidi(int midiNoteNumber) const noexcept;
};
