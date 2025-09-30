#include "SSGEngine.h"
#include <cmath>

namespace
{
    constexpr double twoPi = juce::MathConstants<double>::twoPi;
}

SSGEngine::SSGEngine()
{
    for (size_t i = 0; i < userWaveform.size(); ++i)
        userWaveform[i] = std::sin(juce::MathConstants<float>::twoPi * static_cast<float>(i) / static_cast<float>(userWaveform.size()));
}

void SSGEngine::setSampleRate(double newSampleRate)
{
    sampleRate = juce::jmax(1.0, newSampleRate);
    allNotesOff();
}

void SSGEngine::noteOn(int midiNoteNumber, float velocity)
{
    for (auto& voice : voices)
    {
        if (!voice.active)
        {
            voice.active = true;
            voice.midiNote = midiNoteNumber;
            voice.velocity = velocity;
            voice.phase = 0.0;
            voice.pwm = 0.5f;
            voice.noise = (midiNoteNumber % 16) == 0;
            return;
        }
    }
    voices.front().active = true;
    voices.front().midiNote = midiNoteNumber;
    voices.front().velocity = velocity;
    voices.front().phase = 0.0;
}

void SSGEngine::noteOff(int midiNoteNumber)
{
    for (auto& voice : voices)
        if (voice.active && voice.midiNote == midiNoteNumber)
            voice.active = false;
}

void SSGEngine::allNotesOff()
{
    for (auto& voice : voices)
        voice.active = false;
}

void SSGEngine::render(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    if (buffer.getNumChannels() == 0)
        return;

    for (auto& voice : voices)
    {
        if (!voice.active)
            continue;

        const double frequency = juce::MidiMessage::getMidiNoteInHertz(voice.midiNote);
        const double phaseDelta = twoPi * frequency / sampleRate;

        for (int sample = 0; sample < numSamples; ++sample)
        {
            float value = 0.0f;
            if (voice.noise)
            {
                value = voice.random.nextFloat() * 2.0f - 1.0f;
            }
            else
            {
                double wrapped = std::fmod(voice.phase / twoPi, 1.0);
                size_t index = static_cast<size_t>(wrapped * userWaveform.size()) % userWaveform.size();
                value = userWaveform[index];

                const double duty = voice.pwm;
                const double pulse = (wrapped < duty) ? 1.0 : -1.0;
                value = 0.5f * (value + static_cast<float>(pulse));
            }

            value *= voice.velocity * 0.3f;

            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
                buffer.addSample(ch, startSample + sample, value);

            voice.phase = std::fmod(voice.phase + phaseDelta, twoPi);
        }
    }
}

void SSGEngine::setUserWaveform(const std::array<float, 32>& wave)
{
    userWaveform = wave;
}
