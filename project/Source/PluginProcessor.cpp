#include "PluginProcessor.h"
#include "PluginEditor.h"
MataleTuneAudioProcessor::MataleTuneAudioProcessor()
: AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true)
.withOutput("Output",juce::AudioChannelSet::stereo(),true)),
parameters(*this,nullptr,"MataleTuneState",createLayout()) {
    for(int i=0;i<matale::parameterCount;++i)values[i]=parameters.getRawParameterValue(matale::parameterList[i].id);
}
juce::AudioProcessorValueTreeState::ParameterLayout MataleTuneAudioProcessor::createLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for(int i=0;i<matale::parameterCount;++i) {
        const auto& d=matale::parameterList[i];
        if(i<3) {
            juce::StringArray choices;
            if(i==0)choices={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
            if(i==1)choices={"Chromatic","Major","Minor","Major Pentatonic","Minor Pentatonic"};
            if(i==2)choices={"Natural","Hard Tune"};
            layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{d.id,1},d.name,choices,int(d.initial)));
        } else if(i==3)layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{d.id,1},d.name,true));
        else layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{d.id,1},d.name,
            juce::NormalisableRange<float>(d.lo,d.hi,d.step),d.initial));
    }
    return layout;
}
bool MataleTuneAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const {
    const auto out=l.getMainOutputChannelSet();
    return (out==juce::AudioChannelSet::mono()||out==juce::AudioChannelSet::stereo()) && l.getMainInputChannelSet()==out;
}
void MataleTuneAudioProcessor::prepareToPlay(double rate,int) {
    engine.prepare(rate);setLatencySamples(matale::Engine::latency);
    autoState.store(0);requestAuto.store(false);scopeWrite=scopeDecimate=0;
    for(auto& x:scope)x.store(0);
}
void MataleTuneAudioProcessor::render(juce::AudioBuffer<float>& b,bool bypass) {
    juce::ScopedNoDenormals guard;
    if(b.getNumChannels()<1)return;
    matale::Params p;
    for(int i=0;i<matale::parameterCount;++i)matale::setParam(p,i,values[i]->load(std::memory_order_relaxed));
    if(auto* playhead=getPlayHead())
        if(auto position=playhead->getPosition())
            if(auto bpm=position->getBpm())p.bpm=float(*bpm);
    if(bypass)p.power=0;
    if(requestAuto.exchange(false)) {engine.beginAuto();autoState.store(1);}
    for(int i=0;i<b.getNumSamples();++i)if(++scopeDecimate==64) {
        scopeDecimate=0;scope[size_t(scopeWrite)].store(b.getSample(0,i),std::memory_order_relaxed);
        scopeWrite=(scopeWrite+1)%256;
    }
    engine.process(b.getWritePointer(0),b.getNumChannels()>1?b.getWritePointer(1):nullptr,b.getNumSamples(),p);
    if(engine.autoReady) {
        engine.autoReady=false;
        const auto& a=engine.autoResult;
        const float v[]={a.threshold,a.makeup,a.retune,a.humanize,a.low,a.presence,a.delayMix,a.reverbMix};
        for(int i=0;i<8;++i)proposal[i].store(v[i],std::memory_order_relaxed);
        autoState.store(a.valid?2:3,std::memory_order_release);
    }
    progress.store(engine.autoProgress());displayedPitch.store(engine.pitch());
    inputLevel.store(engine.inputPeak);outputLevel.store(engine.outputPeak);
}
void MataleTuneAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&) {render(b,false);}
void MataleTuneAudioProcessor::processBlockBypassed(juce::AudioBuffer<float>& b,juce::MidiBuffer&) {render(b,true);}
void MataleTuneAudioProcessor::applyAutoResult() {
    if(autoState.load(std::memory_order_acquire)!=2)return;
    const char* ids[]={"threshold","makeup","retune","humanize","eq2g","eq4g","delayMix","reverbMix"};
    for(int i=0;i<8;++i)if(auto* p=parameters.getParameter(ids[i])) {
        p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(proposal[i].load()));p->endChangeGesture();
    }
    autoState.store(0);
}
juce::AudioProcessorEditor* MataleTuneAudioProcessor::createEditor(){return new MataleTuneAudioProcessorEditor(*this);}
void MataleTuneAudioProcessor::getStateInformation(juce::MemoryBlock& b){
    if(auto xml=parameters.copyState().createXml())copyXmlToBinary(*xml,b);
}
void MataleTuneAudioProcessor::setStateInformation(const void* data,int size){
    if(auto xml=getXmlFromBinary(data,size))if(xml->hasTagName(parameters.state.getType()))parameters.replaceState(juce::ValueTree::fromXml(*xml));
}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new MataleTuneAudioProcessor();}
