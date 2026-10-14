#pragma once
#include <JuceHeader.h>
#include "Parameters.h"
class MataleTuneAudioProcessor : public juce::AudioProcessor {
public:
    MataleTuneAudioProcessor();
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
    std::atomic<float> displayedPitch{0},inputLevel{0},outputLevel{0},progress{0};
    std::array<std::atomic<float>,256> scope{};
    std::atomic<bool> requestAuto{false};
    // 0 idle, 1 listening, 2 proposal ready, 3 insufficient voice.
    std::atomic<int> autoState{0};
    void applyAutoResult(); // message thread ONLY
private:
    matale::Engine engine;
    std::array<std::atomic<float>*,matale::parameterCount> values{};
    std::array<std::atomic<float>,8> proposal{};
    int scopeWrite=0,scopeDecimate=0;
    void render(juce::AudioBuffer<float>&,bool);
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MataleTuneAudioProcessor)
};
