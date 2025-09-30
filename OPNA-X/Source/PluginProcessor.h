#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "FMEngine.h"
#include "PCMEngine.h"
#include "SSGEngine.h"

class OpnaXAudioProcessor  : public juce::AudioProcessor
{
public:
    OpnaXAudioProcessor();
    ~OpnaXAudioProcessor() override = default;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    //==============================================================================
    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    FMEngine& getFMEngine() noexcept { return fmEngine; }
    PCMEngine& getPCMEngine() noexcept { return pcmEngine; }
    SSGEngine& getSSGEngine() noexcept { return ssgEngine; }

    juce::AudioProcessorValueTreeState& getValueTreeState() noexcept { return parameters; }

private:
    juce::AudioProcessorValueTreeState parameters;

    FMEngine fmEngine;
    PCMEngine pcmEngine;
    SSGEngine ssgEngine;

    juce::AudioBuffer<float> fmBuffer;
    juce::AudioBuffer<float> pcmBuffer;
    juce::AudioBuffer<float> ssgBuffer;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpnaXAudioProcessor)
};
