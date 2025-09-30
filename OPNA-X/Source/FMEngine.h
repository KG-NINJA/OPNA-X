#pragma once

#include <array>
#include <vector>
#include <optional>
#include <juce_audio_basics/juce_audio_basics.h>

// FMEngine: lightweight 6-operator FM synth core supporting algorithm routing and waveform selection.
class FMEngine
{
public:
    enum class Waveform
    {
        sine = 0,
        square,
        saw,
        triangle
    };

    struct OperatorSettings
    {
        float ratio = 1.0f;
        float outputLevel = 0.5f;
        float modulationIndex = 1.0f;
        Waveform waveform = Waveform::sine;
    };

    struct Algorithm
    {
        // routes[op] == -1 means audible output, otherwise index of operator being modulated
        std::array<int, 6> routes{};
    };

    FMEngine();

    void setSampleRate(double newSampleRate);
    void setAlgorithmIndex(int index);

    void noteOn(int midiNoteNumber, float velocity);
    void noteOff(int midiNoteNumber);
    void allNotesOff();

    void render(juce::AudioBuffer<float>& buffer, int startSample, int numSamples);

    const std::vector<Algorithm>& getAlgorithms() const noexcept { return algorithms; }
    int getAlgorithmIndex() const noexcept { return algorithmIndex; }

private:
    struct OperatorState
    {
        OperatorSettings settings{};
        double phase = 0.0;
    };

    struct Voice
    {
        bool active = false;
        int midiNote = -1;
        float velocity = 0.0f;
        double baseFrequency = 0.0;
        std::array<OperatorState, 6> operators{};
    };

    double sampleRate = 44100.0;
    int algorithmIndex = 0;
    std::vector<Algorithm> algorithms;
    std::array<Voice, 8> voices{};

    Voice* findFreeVoice();
    Voice* findVoice(int midiNoteNumber);

    static float renderWaveform(Waveform type, double phase, float modulation);
};
