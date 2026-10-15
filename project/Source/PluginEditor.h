#pragma once
#include "PluginProcessor.h"
class MataleLookAndFeel final : public juce::LookAndFeel_V4 {
public:
    MataleLookAndFeel();
    void drawButtonBackground(juce::Graphics&,juce::Button&,const juce::Colour&,bool,bool) override;
    void drawButtonText(juce::Graphics&,juce::TextButton&,bool,bool) override;
    void drawComboBox(juce::Graphics&,int,int,bool,int,int,int,int,juce::ComboBox&) override;
    void drawToggleButton(juce::Graphics&,juce::ToggleButton&,bool,bool) override;
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
    juce::Component manualPanel;
    juce::TextButton manualButton{"Manual controls"};
    juce::ComboBox manualPage,factoryPreset,modulePreset;
    juce::TextButton saveButton{"Save"},loadButton{"Load"},defaultButton{"Default"},moduleDefault{"Default"};
    std::array<juce::TextButton,5> pageTabs;
    std::vector<juce::Rectangle<int>> manualModules;
    std::unique_ptr<juce::FileChooser> presetChooser;
    juce::String presetNotice;
    double noticeUntil=0;
    int noticeProfile=-1;
    void choosePreset(bool saving);
    void paintManual(juce::Graphics&);
    struct ManualControl {
        int page;
        std::unique_ptr<juce::Slider> knob;
        std::unique_ptr<juce::ComboBox> choice;
        std::unique_ptr<juce::ToggleButton> toggle;
        std::unique_ptr<juce::Label> label;
    };
    std::vector<ManualControl> manualControls;
    std::vector<std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment>> choices;
    bool manualOpen=false;
    void showManual(bool);
    void layoutManual();
    juce::TextButton autoButton{"AUTO"},peakReset{"Reset peak"};
    juce::ToggleButton power{"On"},harsh{"Harsh control"},autoKey{"Auto detect key (Chromatic only)"};
    std::array<juce::ToggleButton,7> includes;
    std::array<juce::Slider,3> knobs;
    std::array<juce::Label,3> knobLabels;
    juce::Label status,credit;
    juce::HyperlinkButton telegram{"Telegram @kominit",juce::URL("https://t.me/kominit")};
    using BA=juce::AudioProcessorValueTreeState::ButtonAttachment;
    using SA=juce::AudioProcessorValueTreeState::SliderAttachment;
    std::vector<std::unique_ptr<BA>> buttons;
    std::vector<std::unique_ptr<SA>> sliders;
    juce::Rectangle<int> analysisBounds,outputBounds,autoBounds,smoothBounds;
    juce::dsp::FFT fft{11};std::array<float,4096> fftData{};std::array<float,120> spectrum{};
    void timerCallback() override;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MataleTuneAudioProcessorEditor)
};
