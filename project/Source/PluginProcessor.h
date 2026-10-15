#pragma once
#include <JuceHeader.h>
#include "NativeVocal.h"
class MataleTuneAudioProcessor : public juce::AudioProcessor, private juce::Timer {
public:
    MataleTuneAudioProcessor();
    ~MataleTuneAudioProcessor() override;
    void prepareToPlay(double,int) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout&) const override;
    void processBlock(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    void processBlockBypassed(juce::AudioBuffer<float>&,juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override {return true;}
    const juce::String getName() const override {return "Matale Tune";}
    bool acceptsMidi() const override {return false;}
    bool producesMidi() const override {return false;}
    bool isMidiEffect() const override {return false;}
    double getTailLengthSeconds() const override {return 30;}
    int getNumPrograms() override {return 1;}
    int getCurrentProgram() override {return 0;}
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override {return {};}
    void changeProgramName(int,const juce::String&) override {}
    void getStateInformation(juce::MemoryBlock&) override;
    void setStateInformation(const void*,int) override;
    juce::AudioProcessorValueTreeState parameters;
    std::atomic<float> displayedPitch{0},pitchConfidence{0},inputLevel{0},inputRms{0},outputLevel{0},progress{0};
    std::atomic<float> leftLevel{0},rightLevel{0},samplePeak{0},momentaryLufs{-180},smoothReduction{0},deessReduction{0};
    std::atomic<int> minNote{128},maxNote{-1},medianNote{-1},voicedNotes{0};
    std::array<std::atomic<float>,2048> scope{};
    std::atomic<int> scopePosition{0};
    std::atomic<bool> requestAuto{false},requestPeakReset{false};
    // 0 idle, 1 listening, 2 mailbox ready, 3 insufficient voice.
    std::atomic<int> autoState{0},lastAutoStatus{0},detectedRoot{-1},detectedScale{0};
    bool applyFactoryPreset(int profile);
    bool savePreset(const juce::File&,juce::String& error);
    bool loadPreset(const juce::File&,juce::String& error);
    void applyAutoResult(); // message thread; timer works with editor closed
private:
    matale::Engine engine;
    matale::NativeVocal vocal;
    matale::NativeAuto analyser;
    matale::Yin tracker;
    std::array<std::atomic<float>*,matale::parameterCount> values{};
    std::array<std::atomic<float>*,matale::nativeParameterCount> extras{};
    matale::NativeProposal proposal; // single-writer mailbox, protected by autoState
    std::array<int,128> noteCounts{};
    int scopeWrite=0,notesSeen=0,minimum=128,maximum=-1,lastMidi=-1,midiRun=0;
    void render(juce::AudioBuffer<float>&,bool);
    void timerCallback() override;
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MataleTuneAudioProcessor)
};
