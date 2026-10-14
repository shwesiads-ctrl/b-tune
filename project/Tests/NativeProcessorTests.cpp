#include "PluginEditor.h"
#include <iostream>
#include <thread>
#include <stdexcept>
void check(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
void set(MataleTuneAudioProcessor& p,const char* id,float v){auto* parameter=p.parameters.getParameter(id);check(parameter!=nullptr,"Missing native parameter");parameter->setValueNotifyingHost(parameter->convertTo0to1(v));}
float value(MataleTuneAudioProcessor& p,const char* id){return p.parameters.getRawParameterValue(id)->load();}
void layoutCheck(juce::Component& c){for(auto* child:c.getChildren())if(child->isVisible()){check(c.getLocalBounds().contains(child->getBounds()),"Compact UI child outside parent");if(auto* slider=dynamic_cast<juce::Slider*>(child))check(slider->getSliderStyle()==juce::Slider::RotaryHorizontalVerticalDrag,"Plugin has a linear slider");layoutCheck(*child);}}
struct HostListener:juce::AudioProcessorParameter::Listener{std::thread::id message=std::this_thread::get_id();bool wrongThread=false;int changes=0;void parameterValueChanged(int,float)override{wrongThread|=std::this_thread::get_id()!=message;++changes;}void parameterGestureChanged(int,bool)override{wrongThread|=std::this_thread::get_id()!=message;}};
void manualCheck(MataleTuneAudioProcessor& p,juce::AudioProcessorEditor* editor){
    juce::MemoryBlock before;p.getStateInformation(before);
    juce::TextButton* manual=nullptr;for(auto* child:editor->getChildren())if(child->getComponentID()=="manual")manual=dynamic_cast<juce::TextButton*>(child);
    check(manual!=nullptr,"Manual button missing");manual->onClick();
    juce::Component* panel=nullptr;juce::ComboBox* pages=nullptr;
    for(auto* child:editor->getChildren())for(auto* c:child->getChildren())if(c->getComponentID()=="manual-page"){panel=child;pages=dynamic_cast<juce::ComboBox*>(c);}
    check(panel&&panel->isVisible()&&pages,"Manual panel missing");
    int edited=0;
    for(int page=1;page<=5;++page){pages->setSelectedId(page,juce::sendNotificationSync);
        for(auto size:{juce::Point<int>(980,620),juce::Point<int>(1200,720),juce::Point<int>(1800,1000)}){editor->setSize(size.x,size.y);layoutCheck(*editor);}
        for(auto* child:panel->getChildren())if(child->isVisible()){
            auto id=child->getComponentID();auto* param=p.parameters.getParameter(id);if(!param)continue;
            if(auto* knob=dynamic_cast<juce::Slider*>(child)){float target=param->convertFrom0to1(.63f);knob->setValue(target,juce::sendNotificationSync);check(std::abs(value(p,id.toRawUTF8())-float(knob->getValue()))<.011f,"Manual knob does not update processor");++edited;}
            if(auto* choice=dynamic_cast<juce::ComboBox*>(child)){choice->setSelectedId(2,juce::sendNotificationSync);if(value(p,id.toRawUTF8())!=1)std::cerr<<"Selector "<<id<<" value="<<value(p,id.toRawUTF8())<<" selected="<<choice->getSelectedId()<<"\n";check(value(p,id.toRawUTF8())==1,"Manual selector does not update processor");++edited;}
            if(auto* toggle=dynamic_cast<juce::ToggleButton*>(child)){toggle->setToggleState(false,juce::sendNotificationSync);check(value(p,id.toRawUTF8())==0,"Manual toggle does not update processor");++edited;}
        }
    }
    check(edited==48,"Manual parameters missing");
    pages->setSelectedId(3,juce::sendNotificationSync);editor->setSize(1200,720);
    if(auto path=juce::SystemStats::getEnvironmentVariable("MATALE_NATIVE_SCREENSHOT",{});path.isNotEmpty()){
        auto file=juce::File(path).getSiblingFile("Native-Manual-UI.png");file.deleteFile();juce::FileOutputStream out(file);
        check(out.openedOk()&&juce::PNGImageFormat().writeImageToStream(editor->createComponentSnapshot(editor->getLocalBounds()),out),"Cannot save manual preview");
    }
    manual->onClick();check(!panel->isVisible(),"Manual did not return to Auto");editor->setSize(1200,720);layoutCheck(*editor);p.setStateInformation(before.getData(),int(before.getSize()));
}
int main(){try{juce::ScopedJuceInitialiser_GUI init;MataleTuneAudioProcessor p;p.setRateAndBufferSizeDetails(48000,256);p.prepareToPlay(48000,256);check(p.getLatencySamples()==2176,"Incorrect host latency");HostListener listener;for(auto* param:p.getParameters())param->addListener(&listener);
    p.requestAuto.store(true);std::exception_ptr failure;
    std::thread audio([&]{try{juce::AudioBuffer<float> b(2,256);juce::MidiBuffer midi;for(int block=0;block<600;++block){for(int i=0;i<256;++i){int n=block*256+i;float hz=n<72000?261.6256f:n<115200?329.6276f:391.9954f;float x=.15f*std::sin(2*matale::pi*hz*n/48000);b.setSample(0,i,x);b.setSample(1,i,x*.5f);}p.processBlock(b,midi);for(int c=0;c<2;++c)for(int i=0;i<256;++i)check(std::isfinite(b.getSample(c,i))&&std::abs(b.getSample(c,i))<=.950001f,"Invalid native output");}}catch(...){failure=std::current_exception();}});audio.join();if(failure)std::rethrow_exception(failure);check(p.autoState.load()==2,"Native Auto did not publish proposal");check(listener.changes==0,"Auto notified host on audio thread");
    std::unique_ptr<juce::AudioProcessorEditor> editor;bool closedVerified=false;juce::Timer::callAfterDelay(100,[&]{closedVerified=p.autoState.load()==0&&listener.changes>10&&!listener.wrongThread;if(closedVerified){editor.reset(p.createEditor());try{manualCheck(p,editor.get());}catch(...){failure=std::current_exception();}}});
    juce::Timer::callAfterDelay(250,[]{juce::MessageManager::getInstance()->stopDispatchLoop();});juce::MessageManager::getInstance()->runDispatchLoop();if(failure)std::rethrow_exception(failure);check(closedVerified&&editor!=nullptr,"Auto must apply on message thread with editor closed");check(value(p,"key")==0&&value(p,"scale")==1,"Native Auto did not find C Major");check(value(p,"correction")==100&&value(p,"style")==0,"Natural pitch Auto not applied");check(value(p,"vocalSmooth")>0&&value(p,"deessAmount")>0,"Native vocal Auto missing");
    set(p,"vocalWarmth",27);set(p,"vocalPresence",15);set(p,"includeDelay",0);juce::MemoryBlock state;p.getStateInformation(state);MataleTuneAudioProcessor restored;restored.setStateInformation(state.getData(),int(state.getSize()));check(value(restored,"vocalWarmth")==27&&value(restored,"vocalPresence")==15&&value(restored,"includeDelay")==0,"Native state failed");
    auto legacy=p.parameters.copyState();for(int i=legacy.getNumChildren()-1;i>=0;--i){auto child=legacy.getChild(i);bool old=false;for(const auto& d:matale::parameterList)old|=child["id"].toString()==d.id;if(!old)legacy.removeChild(i,nullptr);}juce::MemoryBlock legacyData;if(auto xml=legacy.createXml())juce::AudioProcessor::copyXmlToBinary(*xml,legacyData);restored.setStateInformation(legacyData.getData(),int(legacyData.getSize()));check(value(restored,"vocalWarmth")==0&&value(restored,"outputSafety")==1,"Legacy state migration failed");
    for(auto size:{juce::Point<int>(980,620),juce::Point<int>(1200,720),juce::Point<int>(1800,1000)}){editor->setSize(size.x,size.y);layoutCheck(*editor);}editor->setSize(1200,720);int knobs=0;for(auto* child:editor->getChildren())if(dynamic_cast<juce::Slider*>(child))++knobs;check(knobs==3,"Compact plugin must expose only Smooth/Warmth/Presence knobs");
    if(auto path=juce::SystemStats::getEnvironmentVariable("MATALE_NATIVE_SCREENSHOT",{});path.isNotEmpty()){juce::File file(path);file.deleteFile();juce::FileOutputStream out(file);check(out.openedOk(),"Cannot save UI preview");check(juce::PNGImageFormat().writeImageToStream(editor->createComponentSnapshot(editor->getLocalBounds()),out),"Cannot encode UI preview");}
    auto hostile=p.parameters.copyState();hostile.getChildWithProperty("id","vocalWarmth").setProperty("value",1.e20,nullptr);juce::MemoryBlock hostileData;if(auto xml=hostile.createXml())juce::AudioProcessor::copyXmlToBinary(*xml,hostileData);restored.setStateInformation(hostileData.getData(),int(hostileData.getSize()));check(value(restored,"vocalWarmth")==100,"State range validation failed");
    // Exclusion and manually selected scale survive an Auto pass.
    set(p,"key",9);set(p,"scale",1);set(p,"humanize",87);set(p,"includePitch",0);p.requestAuto.store(true);juce::AudioBuffer<float> block(2,256);juce::MidiBuffer midi;for(int b=0;b<600;++b){for(int i=0;i<256;++i){float x=.1f*std::sin(2*matale::pi*220*(b*256+i)/48000);block.setSample(0,i,x);block.setSample(1,i,x);}p.processBlock(block,midi);}p.applyAutoResult();check(value(p,"key")==9&&value(p,"scale")==1&&value(p,"humanize")==87,"Excluded/manual pitch settings changed");
    p.prepareToPlay(48000,256);p.requestAuto.store(true);for(int b=0;b<600;++b){block.clear();p.processBlock(block,midi);}check(p.autoState.load()==3,"Silence Auto should fail without applying");
    for(auto* param:p.getParameters())param->removeListener(&listener);std::cout<<"PASS actual JUCE processor: closed-editor Auto/key, host notification threading, sample ceiling, state migration, exclusions, silence and compact UI bounds\n";
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
