#pragma once
#include "PluginProcessor.h"

class MataleLookAndFeel final : public juce::LookAndFeel_V4 {
public:
    MataleLookAndFeel();
    void drawButtonBackground(juce::Graphics&, juce::Button&, const juce::Colour&, bool, bool) override;
    void drawButtonText(juce::Graphics&, juce::TextButton&, bool, bool) override;
    void drawRotarySlider(juce::Graphics&, int, int, int, int, float, float, float, juce::Slider&) override;
};

class MataleTuneAudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit MataleTuneAudioProcessorEditor(MataleTuneAudioProcessor&);
    ~MataleTuneAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    MataleTuneAudioProcessor& processor;
    MataleLookAndFeel look;
    juce::TextButton autoButton{"AUTO"}, manualButton{"Manual controls"};
    juce::ToggleButton power{"Processing on"};
    juce::Label status;
    std::array<juce::TextButton, 5> navigation;
    std::array<juce::Component, 4> pages;
    std::array<juce::ComboBox, 3> choices;
    std::array<juce::Label, matale::parameterCount> labels;
    std::array<juce::Slider, matale::parameterCount> sliders;
    std::array<juce::Slider, 6> quickSliders;
    using SA = juce::AudioProcessorValueTreeState::SliderAttachment;
    using CA = juce::AudioProcessorValueTreeState::ComboBoxAttachment;
    std::vector<std::unique_ptr<SA>> sliderAttachments;
    std::vector<std::unique_ptr<CA>> comboAttachments;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> powerAttachment;
    int selectedPage = 0;
    juce::Rectangle<int> analysisBounds, outputBounds, autoBounds, summaryBounds, rackBounds;
    void selectPage(int);
    void timerCallback() override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MataleTuneAudioProcessorEditor)
};
