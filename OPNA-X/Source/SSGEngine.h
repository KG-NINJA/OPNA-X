#pragma once

#include <array>
#include <juce_audio_basics/juce_audio_basics.h>

// SSGEngine: simple PSG-style generator with user waveform, noise, and PWM.
class SSGEngine
{
public:
    SSGEngine();

    void setSampleRate(double newSampleRate);

    void noteOn(int midiNoteNumber, float velocity);
    void noteOff(int midiNoteNumber);
    void allNotesOff();

    void render(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    void setUserWaveform(const std::array<float, 32>& wave);
    const std::array<float, 32>& getUserWaveform() const noexcept { return userWaveform; }

private:
    struct Voice
    {
        bool active = false;
        int midiNote = -1;
        double phase = 0.0;
        float velocity = 0.0f;
        float pwm = 0.5f;
        bool noise = false;
        juce::Random random;
    };

    double sampleRate = 44100.0;
    std::array<Voice, 3> voices{};
    std::array<float, 32> userWaveform{};
};
