#include "FMEngine.h"
#include <cmath>
#include <functional>

namespace
{
    constexpr double twoPi = juce::MathConstants<double>::twoPi;
}

FMEngine::FMEngine()
{
    // DX7-inspired routing sets for the six operators.
    algorithms = {
        Algorithm{ { 1, 2, 3, 4, 5, -1 } }, // cascading stack
        Algorithm{ { 1, -1, 3, -1, -1, -1 } }, // dual carrier pair
        Algorithm{ { -1, -1, -1, -1, -1, -1 } }, // parallel carriers
        Algorithm{ { 5, 5, 5, 5, -1, -1 } }, // multi-mod into two carriers
        Algorithm{ { 1, 2, -1, 5, -1, -1 } } // mixed tower + carrier
    };
}

void FMEngine::setSampleRate(double newSampleRate)
{
    sampleRate = juce::jmax(1.0, newSampleRate);
    allNotesOff();
}

void FMEngine::setAlgorithmIndex(int index)
{
    if (index >= 0 && index < static_cast<int>(algorithms.size()))
        algorithmIndex = index;
}

void FMEngine::noteOn(int midiNoteNumber, float velocity)
{
    if (auto* voice = findVoice(midiNoteNumber))
    {
        voice->velocity = velocity;
        voice->active = true;
        return;
    }

    if (auto* voice = findFreeVoice())
    {
        voice->active = true;
        voice->midiNote = midiNoteNumber;
        voice->velocity = velocity;
        voice->baseFrequency = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);

        for (size_t op = 0; op < voice->operators.size(); ++op)
        {
            auto& state = voice->operators[op];
            state.phase = 0.0;
            state.settings.ratio = 1.0f + static_cast<float>(op) * 0.25f;
            state.settings.outputLevel = 0.7f;
            state.settings.modulationIndex = 1.0f + static_cast<float>(op) * 0.1f;
            state.settings.waveform = Waveform::sine;
        }

        voice->operators[0].settings.waveform = Waveform::sine;
        voice->operators[1].settings.waveform = Waveform::square;
        voice->operators[2].settings.waveform = Waveform::triangle;
        voice->operators[3].settings.waveform = Waveform::saw;
        voice->operators[4].settings.waveform = Waveform::sine;
        voice->operators[5].settings.waveform = Waveform::sine;
    }
}

void FMEngine::noteOff(int midiNoteNumber)
{
    if (auto* voice = findVoice(midiNoteNumber))
        voice->active = false;
}

void FMEngine::allNotesOff()
{
    for (auto& voice : voices)
        voice.active = false;
}

void FMEngine::render(juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    if (buffer.getNumChannels() == 0)
        return;

    auto& algorithm = algorithms[(size_t)juce::jlimit(0, (int)algorithms.size() - 1, algorithmIndex)];

    for (int sample = 0; sample < numSamples; ++sample)
    {
        float mixedSample = 0.0f;

        for (auto& voice : voices)
        {
            if (!voice.active)
                continue;

            std::array<float, 6> opOutputs{};
            std::array<bool, 6> computed{};

            std::function<float(size_t)> renderOperator = [&](size_t opIndex) -> float
            {
                if (computed[opIndex])
                    return opOutputs[opIndex];

                auto& state = voice.operators[opIndex];
                double freq = voice.baseFrequency * state.settings.ratio;
                double phaseAdvance = twoPi * freq / sampleRate;

                float modulation = 0.0f;
                for (size_t modOp = 0; modOp < voice.operators.size(); ++modOp)
                {
                    if (algorithm.routes[modOp] == static_cast<int>(opIndex))
                        modulation += renderOperator(modOp) * voice.operators[modOp].settings.modulationIndex;
                }

                state.phase = std::fmod(state.phase + phaseAdvance, twoPi);
                float raw = renderWaveform(state.settings.waveform, state.phase, modulation);
                opOutputs[opIndex] = raw * state.settings.outputLevel * voice.velocity;
                computed[opIndex] = true;
                return opOutputs[opIndex];
            };

            for (size_t op = 0; op < voice.operators.size(); ++op)
            {
                if (algorithm.routes[op] < 0)
                    mixedSample += renderOperator(op);
            }
        }

        for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            buffer.addSample(ch, startSample + sample, mixedSample);
    }
}

FMEngine::Voice* FMEngine::findFreeVoice()
{
    for (auto& voice : voices)
        if (!voice.active)
            return &voice;
    return &voices.front();
}

FMEngine::Voice* FMEngine::findVoice(int midiNoteNumber)
{
    for (auto& voice : voices)
        if (voice.active && voice.midiNote == midiNoteNumber)
            return &voice;
    return nullptr;
}

float FMEngine::renderWaveform(Waveform type, double phase, float modulation)
{
    switch (type)
    {
        case Waveform::sine:
            return std::sin(phase + modulation);
        case Waveform::square:
            return (std::sin(phase + modulation) >= 0.0) ? 1.0f : -1.0f;
        case Waveform::saw:
        {
            double wrapped = std::fmod(phase / twoPi, 1.0);
            return static_cast<float>(2.0 * wrapped - 1.0);
        }
        case Waveform::triangle:
        {
            double wrapped = std::fmod((phase + modulation) / twoPi, 1.0);
            double tri = 4.0 * std::abs(wrapped - 0.5) - 1.0;
            return static_cast<float>(tri);
        }
        default:
            break;
    }

    return 0.0f;
}
