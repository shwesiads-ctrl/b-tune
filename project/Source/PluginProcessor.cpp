#include "PluginProcessor.h"
#include "PluginEditor.h"
MataleTuneAudioProcessor::MataleTuneAudioProcessor()
: AudioProcessor(BusesProperties().withInput("Input",juce::AudioChannelSet::stereo(),true)
.withOutput("Output",juce::AudioChannelSet::stereo(),true)),
parameters(*this,nullptr,"MataleTuneState",createLayout()) {
    for(int i=0;i<matale::parameterCount;++i)values[size_t(i)]=parameters.getRawParameterValue(matale::parameterList[i].id);
    for(int i=0;i<matale::nativeParameterCount;++i)extras[size_t(i)]=parameters.getRawParameterValue(matale::nativeParameters[i].id);
    startTimerHz(30);
}
MataleTuneAudioProcessor::~MataleTuneAudioProcessor(){stopTimer();}
juce::AudioProcessorValueTreeState::ParameterLayout MataleTuneAudioProcessor::createLayout() {
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    for(int i=0;i<matale::parameterCount;++i){const auto& d=matale::parameterList[i];
        if(i<3){juce::StringArray choices;if(i==0)choices={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};if(i==1)choices={"Chromatic","Major","Minor","Major Pentatonic","Minor Pentatonic"};if(i==2)choices={"Natural","Hard Tune"};layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{d.id,1},d.name,choices,int(d.initial)));}
        else if(i==3)layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{d.id,1},d.name,true));
        else layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{d.id,1},d.name,juce::NormalisableRange<float>(d.lo,d.hi,d.step),d.initial));
    }
    for(const auto& d:matale::nativeParameters){if(juce::String(d.id)=="autoProfile"){layout.add(std::make_unique<juce::AudioParameterChoice>(juce::ParameterID{d.id,1},d.name,juce::StringArray{"Natural","Clear","Warm","Bright","Hard Tune"},0));}else if(d.boolean)layout.add(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{d.id,1},d.name,d.initial>.5f));else layout.add(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{d.id,1},d.name,juce::NormalisableRange<float>(d.lo,d.hi,d.step),d.initial));}
    return layout;
}
bool MataleTuneAudioProcessor::isBusesLayoutSupported(const BusesLayout& l) const {const auto out=l.getMainOutputChannelSet();return (out==juce::AudioChannelSet::mono()||out==juce::AudioChannelSet::stereo())&&l.getMainInputChannelSet()==out;}
void MataleTuneAudioProcessor::prepareToPlay(double rate,int){engine.prepare(rate);vocal.prepare(rate,getTotalNumOutputChannels());analyser.prepare(rate);tracker.prepare(rate);setLatencySamples(matale::Engine::latency+128);displayedPitch.store(0);pitchConfidence.store(0);inputLevel.store(0);inputRms.store(0);outputLevel.store(0);leftLevel.store(0);rightLevel.store(0);samplePeak.store(0);momentaryLufs.store(-180);progress.store(0);autoState.store(0);requestAuto.store(false);lastAutoStatus.store(0);scopeWrite=0;noteCounts.fill(0);minimum=128;maximum=-1;notesSeen=0;lastMidi=-1;midiRun=0;minNote.store(128);maxNote.store(-1);medianNote.store(-1);voicedNotes.store(0);for(auto& x:scope)x.store(0);scopePosition.store(0);}
void MataleTuneAudioProcessor::render(juce::AudioBuffer<float>& b,bool bypass){
    juce::ScopedNoDenormals guard;const int channels=b.getNumChannels();if(channels<1)return;
    std::array<float,matale::parameterCount> raw{};matale::Params p;matale::NativeValues x{};
    for(int i=0;i<matale::parameterCount;++i){raw[size_t(i)]=values[size_t(i)]->load(std::memory_order_relaxed);matale::setParam(p,i,raw[size_t(i)]);}for(int i=0;i<matale::nativeParameterCount;++i)x[size_t(i)]=extras[size_t(i)]->load(std::memory_order_relaxed);
    if(auto* playhead=getPlayHead())if(auto pos=playhead->getPosition())if(auto bpm=pos->getBpm())if(std::isfinite(*bpm))p.bpm=float(*bpm);
    int state=autoState.load(std::memory_order_acquire);if(requestAuto.exchange(false)&&state!=1&&state!=2){analyser.start();autoState.store(1,std::memory_order_release);lastAutoStatus.store(0);state=1;}
    if(state==1)p.power=1;if(bypass)p.power=0;
    if(requestPeakReset.exchange(false))vocal.resetPeak();vocal.beginBlock();float peak=0;double sum=0;
    auto* left=b.getWritePointer(0);auto* right=channels>1?b.getWritePointer(1):nullptr;
    for(int i=0;i<b.getNumSamples();++i){float l=matale::clean(left[i]),r=right?matale::clean(right[i]):l;peak=std::max(peak,std::max(std::abs(l),std::abs(r)));sum+=(double(l)*l+double(r)*r)*.5;analyser.push(l,r);if(tracker.push((l+r)*.5f)){analyser.pitch(tracker.hz,tracker.confidence);if(tracker.hz>0&&tracker.confidence>=.75f){int note=std::clamp(int(std::round(69+12*std::log2(tracker.hz/440))),0,127);if(lastMidi==note)++midiRun;else{lastMidi=note;midiRun=1;}if(midiRun>=3){minimum=std::min(minimum,note);maximum=std::max(maximum,note);++noteCounts[size_t(note)];++notesSeen;}}}vocal.input(l,r,p,x);left[i]=vocal.left;if(right)right[i]=vocal.right;}
    auto core=p;if(x[matale::Opto]>.5f&&p.ratio>1){core.ratio=1;core.makeup=0;}engine.process(left,right,b.getNumSamples(),core);
    for(int i=0;i<b.getNumSamples();++i){vocal.output(left[i],right?right[i]:left[i],p,x);left[i]=vocal.left;if(right)right[i]=vocal.right;scope[size_t(scopeWrite)].store(left[i],std::memory_order_relaxed);scopeWrite=(scopeWrite+1)%int(scope.size());}
    scopePosition.store(scopeWrite,std::memory_order_release);displayedPitch.store(tracker.hz);pitchConfidence.store(tracker.confidence);inputLevel.store(peak);inputRms.store(float(std::sqrt(sum/std::max(1,b.getNumSamples()))));outputLevel.store(std::max(vocal.leftPeak,vocal.rightPeak));leftLevel.store(vocal.leftPeak);rightLevel.store(right?vocal.rightPeak:0);samplePeak.store(vocal.heldPeak);momentaryLufs.store(vocal.lufs());smoothReduction.store(vocal.smoothReduction);deessReduction.store(vocal.deessReduction*(p.power>.5f?1.f:0.f));minNote.store(minimum);maxNote.store(maximum);voicedNotes.store(notesSeen);
    int median=-1;if(notesSeen>=30&&maximum-minimum>=7){int accumulated=0;for(int i=0;i<128;++i){accumulated+=noteCounts[size_t(i)];if(accumulated>=notesSeen/2){median=i;break;}}}medianNote.store(median);
    progress.store(analyser.progress());if(analyser.ready()){proposal=analyser.finish(raw,x);if(proposal.valid)autoState.store(2,std::memory_order_release);else{lastAutoStatus.store(3);autoState.store(3,std::memory_order_release);}}
}
void MataleTuneAudioProcessor::processBlock(juce::AudioBuffer<float>& b,juce::MidiBuffer&){render(b,false);}
void MataleTuneAudioProcessor::processBlockBypassed(juce::AudioBuffer<float>& b,juce::MidiBuffer&){render(b,true);}
void MataleTuneAudioProcessor::timerCallback(){applyAutoResult();}
void MataleTuneAudioProcessor::applyAutoResult(){if(autoState.load(std::memory_order_acquire)!=2)return;
    auto set=[this](const char* id,float value){if(auto* p=parameters.getParameter(id)){p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(value));p->endChangeGesture();}};
    for(int i=0;i<matale::parameterCount;++i)if(proposal.coreChanged[size_t(i)])set(matale::parameterList[i].id,proposal.core[size_t(i)]);
    for(int i=0;i<matale::nativeParameterCount;++i)if(proposal.extraChanged[size_t(i)])set(matale::nativeParameters[i].id,proposal.extra[size_t(i)]);
    bool appliedKey=proposal.coreChanged[1];detectedRoot.store(appliedKey?proposal.key.root:-1);detectedScale.store(appliedKey?proposal.key.scale:0);lastAutoStatus.store(appliedKey?1:proposal.extra[matale::AutoKey]>.5f&&proposal.extra[matale::IncludePitch]>.5f&&int(proposal.core[1])==0?2:1);autoState.store(0,std::memory_order_release);
}
juce::AudioProcessorEditor* MataleTuneAudioProcessor::createEditor(){return new MataleTuneAudioProcessorEditor(*this);}
void MataleTuneAudioProcessor::getStateInformation(juce::MemoryBlock& b){if(auto xml=parameters.copyState().createXml())copyXmlToBinary(*xml,b);}
void MataleTuneAudioProcessor::setStateInformation(const void* data,int size){
    if(auto xml=getXmlFromBinary(data,size))if(xml->hasTagName(parameters.state.getType())){
        auto tree=juce::ValueTree::fromXml(*xml);
        auto ensure=[&](const char* id,float lo,float hi,float step,float initial){auto child=tree.getChildWithProperty("id",id);if(!child.isValid()){child=juce::ValueTree("PARAM");child.setProperty("id",id,nullptr);tree.appendChild(child,nullptr);}float v=child.hasProperty("value")?float(child["value"]):initial;if(!std::isfinite(v))v=initial;v=std::clamp(v,lo,hi);v=lo+std::round((v-lo)/step)*step;child.setProperty("value",std::clamp(v,lo,hi),nullptr);};
        for(const auto& d:matale::parameterList)ensure(d.id,d.lo,d.hi,d.step,d.initial);
        for(const auto& d:matale::nativeParameters)ensure(d.id,d.lo,d.hi,d.step,d.initial);
        parameters.replaceState(tree);
    }
}

bool MataleTuneAudioProcessor::applyFactoryPreset(int profile){
    if(profile<0||profile>4||requestAuto.load()||autoState.load()==1||autoState.load()==2)return false;
    matale::NativeProposal preset;
    for(int i=0;i<matale::parameterCount;++i){preset.core[size_t(i)]=matale::parameterList[i].initial;preset.coreChanged[size_t(i)]=i==2||(i>=4&&i<=32);}
    preset.extra=matale::nativeDefaults();
    for(int i=0;i<=matale::Safety;++i)preset.extraChanged[size_t(i)]=true;
    preset.core[5]=35;preset.core[31]=.04f;preset.core[28]=.02f;
    matale::applyCharacter(preset,profile);
    auto set=[this](const char* id,float v){auto* p=parameters.getParameter(id);p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(v));p->endChangeGesture();};
    for(int i=0;i<matale::parameterCount;++i)if(preset.coreChanged[size_t(i)])set(matale::parameterList[i].id,preset.core[size_t(i)]);
    for(int i=0;i<=matale::Safety;++i)set(matale::nativeParameters[i].id,preset.extra[size_t(i)]);
    set("autoProfile",float(profile));lastAutoStatus.store(0);detectedRoot.store(-1);return true;
}
bool MataleTuneAudioProcessor::savePreset(const juce::File& file,juce::String& error){
    auto root=std::make_unique<juce::XmlElement>("MataleTunePreset");root->setAttribute("version",1);
    root->addChildElement(parameters.copyState().createXml().release());
    juce::TemporaryFile temporary(file);
    if(!root->writeTo(temporary.getFile())||!temporary.overwriteTargetFileWithTemporary()){
        error="Could not save preset. Choose a writable folder.";return false;
    }
    error.clear();return true;
}
bool MataleTuneAudioProcessor::loadPreset(const juce::File& file,juce::String& error){
    auto fail=[&](const juce::String& message){error=message;return false;};
    if(requestAuto.load()||autoState.load()==1||autoState.load()==2)return fail("Wait for Auto analysis to finish before loading a preset.");
    if(!file.existsAsFile()||file.getSize()>262144)return fail("Preset is missing or too large.");
    auto xml=juce::XmlDocument::parse(file);
    if(!xml||!xml->hasTagName("MataleTunePreset")||xml->getIntAttribute("version")!=1||xml->getNumChildElements()!=1)return fail("Not a supported Matale Tune preset.");
    auto* state=xml->getFirstChildElement();
    if(!state->hasTagName("MataleTuneState"))return fail("Preset parameter data is invalid.");
    auto tree=juce::ValueTree::fromXml(*state);
    if(tree.getNumChildren()!=matale::parameterCount+matale::nativeParameterCount)return fail("Preset parameters are incomplete.");
    std::vector<std::pair<juce::RangedAudioParameter*,float>> validated;
    auto validate=[&](const char* id,float lo,float hi,float step){
        int found=0;double v=0;
        for(auto c:tree)if(c["id"].toString()==id){++found;
            try{size_t end=0;auto raw=c["value"].toString().toStdString();v=std::stod(raw,&end);if(end!=raw.size())return false;}catch(...){return false;}
        }
        if(found!=1||!std::isfinite(v)||v<lo-.00001||v>hi+.00001||std::abs((v-lo)/step-std::round((v-lo)/step))>.002)return false;
        validated.emplace_back(parameters.getParameter(id),float(v));return true;
    };
    for(const auto& d:matale::parameterList)if(!validate(d.id,d.lo,d.hi,d.step))return fail("Preset contains invalid or missing parameter values.");
    for(const auto& d:matale::nativeParameters)if(!validate(d.id,d.lo,d.hi,d.step))return fail("Preset contains invalid or missing vocal settings.");
    for(auto& entry:validated){auto* p=entry.first;p->beginChangeGesture();p->setValueNotifyingHost(p->convertTo0to1(entry.second));p->endChangeGesture();}
    lastAutoStatus.store(0);detectedRoot.store(-1);error.clear();return true;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new MataleTuneAudioProcessor();}
