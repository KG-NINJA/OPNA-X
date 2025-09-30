#include "PluginEditor.h"

WaveformDisplay::WaveformDisplay(OpnaXAudioProcessor& processorRef)
    : processor(processorRef)
{
    startTimerHz(30);
}

WaveformDisplay::~WaveformDisplay()
{
    stopTimer();
}

void WaveformDisplay::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white.withAlpha(0.2f));
    g.drawRect(getLocalBounds());

    auto area = getLocalBounds().toFloat();

    const auto& waveform = processor.getSSGEngine().getUserWaveform();
    if (waveform.empty())
        return;

    juce::Path path;
    const float step = area.getWidth() / static_cast<float>(waveform.size());
    path.startNewSubPath(area.getX(), area.getCentreY());

    for (size_t i = 0; i < waveform.size(); ++i)
    {
        float x = area.getX() + static_cast<float>(i) * step;
        float y = area.getCentreY() - waveform[i] * area.getHeight() * 0.4f;
        path.lineTo(x, y);
    }

    g.setColour(juce::Colours::cyan);
    g.strokePath(path, juce::PathStrokeType(2.0f));
}

void WaveformDisplay::timerCallback()
{
    repaint();
}

OpnaXAudioProcessorEditor::OpnaXAudioProcessorEditor (OpnaXAudioProcessor& p, juce::AudioProcessorValueTreeState& vts)
    : AudioProcessorEditor (&p), processor(p), valueTreeState(vts), waveformDisplay(p)
{
    setSize (600, 380);

    // FM GUI controls (FM routing selection and level control).
    if (auto* param = valueTreeState.getParameter("fmAlgorithm"))
        algorithmBox.addItemList(param->getAllValueStrings(), 1);
    addAndMakeVisible(algorithmBox);
    algorithmAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(valueTreeState, "fmAlgorithm", algorithmBox);

    auto prepareSlider = [this](juce::Slider& slider)
    {
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 80, 20);
        addAndMakeVisible(slider);
    };

    prepareSlider(fmLevelSlider);
    prepareSlider(pcmLevelSlider);
    prepareSlider(ssgLevelSlider);

    fmLevelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(valueTreeState, "fmLevel", fmLevelSlider);
    pcmLevelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(valueTreeState, "pcmLevel", pcmLevelSlider);
    ssgLevelAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(valueTreeState, "ssgLevel", ssgLevelSlider);

    // PCM file exposure for quick inspection.
    const auto labels = processor.getPCMEngine().getChannelLabels();
    for (auto text : labels)
    {
        auto* label = pcmLabels.add(new juce::Label());
        label->setText(text + " : demo_sample.wav", juce::dontSendNotification);
        label->setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(label);
    }

    addAndMakeVisible(waveformDisplay);
}

OpnaXAudioProcessorEditor::~OpnaXAudioProcessorEditor() = default;

void OpnaXAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkslategrey);
    g.setColour(juce::Colours::white);
    g.setFont(18.0f);
    g.drawText("OPNA-X", 10, 10, 200, 24, juce::Justification::centredLeft);

    g.setFont(14.0f);
    g.drawText("FM Algorithm", 20, 50, 120, 20, juce::Justification::centredLeft);
    g.drawText("FM Level", 20, 120, 120, 20, juce::Justification::centredLeft);
    g.drawText("PCM Level", 20, 170, 120, 20, juce::Justification::centredLeft);
    g.drawText("SSG Level", 20, 220, 120, 20, juce::Justification::centredLeft);
    g.drawText("SSG Waveform", 20, 260, 120, 20, juce::Justification::centredLeft);
}

void OpnaXAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced(16);

    auto waveformArea = bounds.removeFromBottom(150);
    waveformDisplay.setBounds(waveformArea);

    auto rightColumn = bounds.removeFromRight(bounds.getWidth() / 2);
    auto leftColumn = bounds;

    auto headerArea = leftColumn.removeFromTop(40);
    juce::ignoreUnused(headerArea);

    auto algorithmArea = leftColumn.removeFromTop(40);
    algorithmBox.setBounds(algorithmArea.removeFromRight(220));

    auto fmLevelArea = leftColumn.removeFromTop(40);
    fmLevelSlider.setBounds(fmLevelArea.removeFromRight(220));

    auto pcmLevelArea = leftColumn.removeFromTop(40);
    pcmLevelSlider.setBounds(pcmLevelArea.removeFromRight(220));

    auto ssgLevelArea = leftColumn.removeFromTop(40);
    ssgLevelSlider.setBounds(ssgLevelArea.removeFromRight(220));

    const int rowHeight = 20;
    for (int i = 0; i < pcmLabels.size(); ++i)
    {
        auto row = rightColumn.removeFromTop(rowHeight);
        pcmLabels[i]->setBounds(row.withHeight(rowHeight));
    }
}
