#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "PluginProcessor.h"

class WaveformDisplay : public juce::Component, private juce::Timer
{
public:
    explicit WaveformDisplay(OpnaXAudioProcessor& processorRef);
    ~WaveformDisplay() override;

    void paint(juce::Graphics& g) override;

private:
    void timerCallback() override;

    OpnaXAudioProcessor& processor;
};

class OpnaXAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    OpnaXAudioProcessorEditor (OpnaXAudioProcessor&, juce::AudioProcessorValueTreeState& vts);
    ~OpnaXAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    OpnaXAudioProcessor& processor;
    juce::AudioProcessorValueTreeState& valueTreeState;

    juce::ComboBox algorithmBox;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> algorithmAttachment;

    juce::Slider fmLevelSlider;
    juce::Slider pcmLevelSlider;
    juce::Slider ssgLevelSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fmLevelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> pcmLevelAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> ssgLevelAttachment;

    juce::OwnedArray<juce::Label> pcmLabels;

    WaveformDisplay waveformDisplay;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpnaXAudioProcessorEditor)
};
