#pragma once
#include "PluginProcessor.h"
class MataleLookAndFeel final : public juce::LookAndFeel_V4 {
public:
    MataleLookAndFeel();
    void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool) override;
    void drawButtonText(juce::Graphics&,juce::TextButton&,bool,bool) override;
    void drawRotarySlider(juce::Graphics&,int,int,int,int,float,float,float,juce::Slider&) override;
};
class MataleTuneAudioProcessorEditor : public juce::AudioProcessorEditor,private juce::Timer {
public:
    explicit MataleTuneAudioProcessorEditor(MataleTuneAudioProcessor&);
    ~MataleTuneAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    MataleTuneAudioProcessor& processor;MataleLookAndFeel look;
    juce::TextButton autoButton{"AUTO"},peakReset{"Reset peak"};
    juce::ToggleButton power{"On"},harsh{"Harsh control"},autoKey{"Auto detect key (Chromatic only)"};
    std::array<juce::ToggleButton,7> includes;
    std::array<juce::Slider,3> knobs;
    std::array<juce::Label,3> knobLabels;
    juce::Label status;
    using BA=juce::AudioProcessorValueTreeState::ButtonAttachment;
    using SA=juce::AudioProcessorValueTreeState::SliderAttachment;
    std::vector<std::unique_ptr<BA>> buttons;
    std::vector<std::unique_ptr<SA>> sliders;
    juce::Rectangle<int> analysisBounds,outputBounds,autoBounds,smoothBounds;
    juce::dsp::FFT fft{11};std::array<float,4096> fftData{};std::array<float,120> spectrum{};
    void timerCallback() override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MataleTuneAudioProcessorEditor)
};
