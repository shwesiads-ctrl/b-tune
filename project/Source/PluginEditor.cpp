#include "PluginEditor.h"
namespace {
const juce::Colour ink(0xff102d53), muted(0xff7386a4), blue(0xff1685ff), cyan(0xff52d8f5);
const juce::Colour line(0xffe3edf6), green(0xff13cda0);
void text(juce::Graphics& g, const juce::String& s, juce::Rectangle<int> bounds,
          float size = 14.f, juce::Colour colour = ink, bool bold = false,
          juce::Justification justify = juce::Justification::centredLeft) {
    g.setColour(colour); g.setFont(juce::FontOptions(size, bold ? juce::Font::bold : juce::Font::plain));
    g.drawText(s, bounds, justify, true);
}
void card(juce::Graphics& g, juce::Rectangle<int> bounds) {
    juce::DropShadow(juce::Colour(0x0c386b9d), 12, {0, 4}).drawForRectangle(g, bounds.reduced(3));
    g.setGradientFill(juce::ColourGradient(juce::Colours::white, float(bounds.getX()), float(bounds.getY()),
                       juce::Colour(0xfff8fbff), float(bounds.getRight()), float(bounds.getBottom()), false));
    g.fillRoundedRectangle(bounds.toFloat(), 16.f);
    g.setColour(juce::Colour(0xffedf3f9)); g.drawRoundedRectangle(bounds.toFloat(), 16.f, 1.f);
}
void meter(juce::Graphics& g, juce::Rectangle<float> r, float peak) {
    g.setColour(juce::Colour(0xffe8eff6)); g.fillRoundedRectangle(r, r.getHeight() / 2);
    float value = juce::jlimit(0.f, 1.f, (matale::db(peak) + 60.f) / 60.f);
    auto filled = r.withWidth(r.getWidth() * value);
    g.setGradientFill(juce::ColourGradient(green, r.getX(), r.getY(), cyan, r.getRight(), r.getY(), false));
    g.fillRoundedRectangle(filled, r.getHeight() / 2);
}
}

MataleLookAndFeel::MataleLookAndFeel() {
    setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xfff3f8fd));
    setColour(juce::Label::textColourId, ink);
    setColour(juce::Slider::textBoxTextColourId, ink);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour(juce::Slider::trackColourId, blue);
    setColour(juce::Slider::backgroundColourId, line);
    setColour(juce::Slider::thumbColourId, blue);
    setColour(juce::ComboBox::backgroundColourId, juce::Colours::white);
    setColour(juce::ComboBox::textColourId, ink);
    setColour(juce::ComboBox::outlineColourId, line);
    setColour(juce::ComboBox::arrowColourId, muted);
    setColour(juce::PopupMenu::backgroundColourId, juce::Colours::white);
    setColour(juce::PopupMenu::textColourId, ink);
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(0xffe5f3ff));
    setColour(juce::PopupMenu::highlightedTextColourId, blue);
    setColour(juce::ToggleButton::textColourId, ink);
    setColour(juce::ToggleButton::tickColourId, blue);
    setColour(juce::TextButton::textColourOffId, muted);
    setColour(juce::TextButton::textColourOnId, blue);
}
void MataleLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& b,
                                             const juce::Colour&, bool over, bool down) {
    auto r = b.getLocalBounds().toFloat().reduced(1);
    if (b.getComponentID() == "auto-orb") {
        r = r.withSizeKeepingCentre(juce::jmin(r.getWidth(), r.getHeight()), juce::jmin(r.getWidth(), r.getHeight()));
        g.setColour(juce::Colour(0xffe2f5ff)); g.fillEllipse(r);
        g.setColour(cyan.withAlpha(.6f)); g.drawEllipse(r.reduced(3), 6);
        auto inner = r.reduced(14);
        g.setGradientFill(juce::ColourGradient(cyan, inner.getX(), inner.getY(),
                          juce::Colour(down ? 0xff0044d0 : 0xff0755ef), inner.getCentreX(), inner.getBottom(), false));
        g.fillEllipse(inner);
        g.setColour(juce::Colours::white.withAlpha(.9f)); g.drawEllipse(inner.reduced(1), 2);
    } else {
        g.setColour(b.getToggleState() ? juce::Colour(0xffe6f3ff) : over ? juce::Colour(0xffedf5fc) : juce::Colour(0xfff8fbfe));
        g.fillRoundedRectangle(r, 11);
        if (b.getComponentID() == "manual") { g.setColour(line); g.drawRoundedRectangle(r, 11, 1); }
    }
}
void MataleLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& b, bool over, bool down) {
    if (b.getComponentID() != "auto-orb") {
        juce::LookAndFeel_V4::drawButtonText(g, b, over, down); return;
    }
    auto r=b.getLocalBounds();
    const int cx=r.getCentreX(), cy=r.getCentreY();
    juce::Path sparkle;
    sparkle.startNewSubPath(float(cx),float(cy-62));
    sparkle.lineTo(float(cx+5),float(cy-49)); sparkle.lineTo(float(cx+18),float(cy-44));
    sparkle.lineTo(float(cx+5),float(cy-39)); sparkle.lineTo(float(cx),float(cy-26));
    sparkle.lineTo(float(cx-5),float(cy-39)); sparkle.lineTo(float(cx-18),float(cy-44));
    sparkle.lineTo(float(cx-5),float(cy-49)); sparkle.closeSubPath();
    g.setColour(juce::Colours::white.withAlpha(b.isEnabled()?1.f:.6f)); g.fillPath(sparkle);
    text(g,b.getButtonText(),{0,cy-14,r.getWidth(),38},b.isEnabled()?27.f:19.f,juce::Colours::white,true,juce::Justification::centred);
    text(g,"3 sec vocal analysis",{0,cy+28,r.getWidth(),22},11.f,juce::Colours::white.withAlpha(.8f),false,juce::Justification::centred);
}
void MataleLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                         float position, float start, float end, juce::Slider&) {
    auto r = juce::Rectangle<float>(float(x), float(y), float(w), float(h)).reduced(9);
    float radius = juce::jmin(r.getWidth(), r.getHeight()) * .5f;
    auto centre = r.getCentre();
    juce::Path track, value;
    track.addCentredArc(centre.x, centre.y, radius, radius, 0, start, end, true);
    value.addCentredArc(centre.x, centre.y, radius, radius, 0, start, start + position * (end-start), true);
    g.setColour(line); g.strokePath(track, juce::PathStrokeType(5, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setColour(blue); g.strokePath(value, juce::PathStrokeType(5, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    auto disc = juce::Rectangle<float>(radius*1.65f, radius*1.65f).withCentre(centre);
    g.setGradientFill(juce::ColourGradient(juce::Colours::white, disc.getX(), disc.getY(),
                       juce::Colour(0xffe9f1f9), disc.getRight(), disc.getBottom(), false));
    g.fillEllipse(disc);
    float angle = start + position * (end-start);
    g.setColour(blue);
    g.drawLine(centre.x, centre.y, centre.x + std::sin(angle)*radius*.65f,
               centre.y - std::cos(angle)*radius*.65f, 2.5f);
    g.fillEllipse(centre.x-3, centre.y-3, 6, 6);
}

namespace {
juce::String noteName(int midi){if(midi<0||midi>127)return "--";const char* names[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};return juce::String(names[midi%12])+juce::String(midi/12-1);}
juce::String decibel(float level){return level>1.e-9f?juce::String(matale::db(level),1):"-inf";}
}
MataleTuneAudioProcessorEditor::MataleTuneAudioProcessorEditor(MataleTuneAudioProcessor& p):AudioProcessorEditor(&p),processor(p){
    setLookAndFeel(&look);addAndMakeVisible(autoButton);autoButton.setComponentID("auto-orb");autoButton.onClick=[this]{processor.requestAuto.store(true);autoButton.setEnabled(false);};autoButton.setTooltip("Play a dry vocal in the DAW, then press Auto. It listens for 3 seconds.");
    addAndMakeVisible(power);addAndMakeVisible(harsh);addAndMakeVisible(autoKey);addAndMakeVisible(peakReset);addAndMakeVisible(status);
    buttons.push_back(std::make_unique<BA>(p.parameters,"auto",power));buttons.push_back(std::make_unique<BA>(p.parameters,"harshControl",harsh));buttons.push_back(std::make_unique<BA>(p.parameters,"autoKey",autoKey));
    const char* includeNames[]={"Vocal Smooth / quiet noise","Dynamic range control","Harsh voice / De-Esser","Smart EQ","Pitch retune","Reverb","Delay"};
    for(int i=0;i<7;++i){addAndMakeVisible(includes[size_t(i)]);includes[size_t(i)].setButtonText(includeNames[i]);buttons.push_back(std::make_unique<BA>(p.parameters,matale::nativeParameters[matale::IncludeSmooth+i].id,includes[size_t(i)]));}
    const char* ids[]={"vocalSmooth","vocalWarmth","vocalPresence"};const char* labels[]={"Smooth amount","Vocal warmth","Presence"};
    for(int i=0;i<3;++i){auto& knob=knobs[size_t(i)];addAndMakeVisible(knob);addAndMakeVisible(knobLabels[size_t(i)]);knobLabels[size_t(i)].setText(labels[i],juce::dontSendNotification);knobLabels[size_t(i)].setFont(juce::FontOptions(11.f));knobLabels[size_t(i)].setJustificationType(juce::Justification::centred);knob.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);knob.setTextBoxStyle(juce::Slider::TextBoxBelow,false,72,22);knob.setTextValueSuffix("%");knob.setTitle(labels[i]);knob.setDoubleClickReturnValue(true,matale::nativeParameters[i==0?matale::Smooth:i==1?matale::Warmth:matale::Presence].initial);sliders.push_back(std::make_unique<SA>(p.parameters,ids[i],knob));}
    peakReset.onClick=[this]{processor.requestPeakReset.store(true);};status.setFont(juce::FontOptions(12.f));status.setColour(juce::Label::textColourId,muted);status.setText("Matale Tune 0.9.0 DAW preview | Play a vocal and press AUTO.",juce::dontSendNotification);
    addAndMakeVisible(manualButton);manualButton.setComponentID("manual");
    manualButton.onClick=[this]{showManual(!manualOpen);};
    addChildComponent(manualPanel);manualPanel.addAndMakeVisible(manualPage);
    manualPage.setComponentID("manual-page");
    manualPage.addItemList({"Pitch / Key", "Compressor", "EQ", "Delay / Reverb / Output", "Vocal / Denoise"},1);
    manualPage.setSelectedId(1);manualPage.onChange=[this]{layoutManual();};
    auto addControl=[this,&p](const char* id,const char* name,float initial,int page,bool boolean,juce::StringArray options={}){
        ManualControl control;control.page=page;
        control.label=std::make_unique<juce::Label>();control.label->setText(name,juce::dontSendNotification);
        control.label->setJustificationType(juce::Justification::centred);manualPanel.addAndMakeVisible(*control.label);
        if(!options.isEmpty()){
            control.choice=std::make_unique<juce::ComboBox>();control.choice->addItemList(options,1);
            control.choice->setComponentID(id);manualPanel.addAndMakeVisible(*control.choice);
            choices.push_back(std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(p.parameters,id,*control.choice));
        }else if(boolean){
            control.toggle=std::make_unique<juce::ToggleButton>("Enabled");control.toggle->setComponentID(id);
            manualPanel.addAndMakeVisible(*control.toggle);buttons.push_back(std::make_unique<BA>(p.parameters,id,*control.toggle));
        }else{
            control.knob=std::make_unique<juce::Slider>();control.knob->setComponentID(id);control.knob->setTitle(name);
            manualPanel.addAndMakeVisible(*control.knob);
            control.knob->setColour(juce::Slider::textBoxTextColourId,ink);
            control.knob->setColour(juce::Slider::textBoxBackgroundColourId,juce::Colours::white);
            control.knob->setColour(juce::Slider::textBoxOutlineColourId,line);
            control.knob->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
            control.knob->setTextBoxStyle(juce::Slider::TextBoxBelow,false,95,24);
            control.knob->setDoubleClickReturnValue(true,initial);
            sliders.push_back(std::make_unique<SA>(p.parameters,id,*control.knob));
        }
        manualControls.push_back(std::move(control));
    };
    for(int i=0;i<matale::parameterCount;++i){const auto& d=matale::parameterList[i];if(i==3)continue;
        juce::StringArray options;
        if(i==0)options={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
        if(i==1)options={"Chromatic","Major","Minor","Major pentatonic","Minor pentatonic"};
        if(i==2)options={"Natural","Hard Tune"};
        addControl(d.id,d.name,d.initial,d.page,false,options);
    }
    for(int i=0;i<=matale::Safety;++i){const auto& d=matale::nativeParameters[i];
        int page=(i==matale::Opto||i==matale::OptoMix||i==matale::SidechainHz||i==matale::Limit)?1: i==matale::Safety?3:4;
        addControl(d.id,d.name,d.initial,page,d.boolean);
    }
    setResizable(true,true);setResizeLimits(980,620,1800,1000);setSize(1200,720);startTimerHz(24);
}
MataleTuneAudioProcessorEditor::~MataleTuneAudioProcessorEditor(){stopTimer();setLookAndFeel(nullptr);}
void MataleTuneAudioProcessorEditor::resized(){manualButton.setBounds(getWidth()-174,getHeight()-29,160,24);manualPanel.setBounds(12,12,getWidth()-24,getHeight()-48);layoutManual();const int pad=12,gap=16,w=getWidth()-pad*2,left=int((w-gap)*.64f),top=juce::jlimit(225,300,int((getHeight()-48)*.40));analysisBounds={pad,pad,left,top};outputBounds={pad+left+gap,pad,w-left-gap,top};autoBounds={pad,top+pad+gap,left,getHeight()-top-pad-gap-36};smoothBounds={outputBounds.getX(),autoBounds.getY(),outputBounds.getWidth(),autoBounds.getHeight()};status.setBounds(pad,getHeight()-28,w-170,22);power.setBounds(outputBounds.getRight()-75,outputBounds.getY()+16,65,25);peakReset.setBounds(outputBounds.getX()+outputBounds.getWidth()/2-35,outputBounds.getBottom()-54,94,24);
    autoKey.setBounds(autoBounds.getX()+18,autoBounds.getY()+68,juce::jmin(310,autoBounds.getWidth()-230),24);for(int i=0;i<7;++i)includes[size_t(i)].setBounds(autoBounds.getX()+18,autoBounds.getY()+103+i*26,juce::jmin(290,autoBounds.getWidth()-230),23);int orb=juce::jlimit(190,260,autoBounds.getWidth()/3);autoButton.setBounds(autoBounds.getRight()-orb-18,autoBounds.getCentreY()-orb/2,orb,orb);
    int cell=(smoothBounds.getWidth()-28)/4,y=smoothBounds.getY()+110,h=juce::jmin(104,smoothBounds.getHeight()-210);const int cols[]={0,2,3};for(int i=0;i<3;++i){int x=smoothBounds.getX()+14+cols[i]*cell;knobLabels[size_t(i)].setBounds(x,y-25,cell,24);knobs[size_t(i)].setBounds(x,y,cell,h);}harsh.setBounds(smoothBounds.getX()+14+cell,y+10,cell,44);
}
void MataleTuneAudioProcessorEditor::showManual(bool open){
    manualOpen=open;
    for(auto* child:getChildren())if(child!=&manualPanel&&child!=&manualButton&&child!=&status)child->setVisible(!open);
    manualPanel.setVisible(open);manualButton.setButtonText(open?"Back to Auto":"Manual controls");
    layoutManual();repaint();
}
void MataleTuneAudioProcessorEditor::layoutManual(){
    manualPage.setBounds(20,52,300,32);
    int page=manualPage.getSelectedId()-1,count=0;
    for(auto& c:manualControls)if(c.page==page)++count;
    const int columns=page==2?5:4,rows=juce::jmax(1,(count+columns-1)/columns);
    int cellW=(manualPanel.getWidth()-40)/columns,cellH=(manualPanel.getHeight()-116)/rows,index=0;
    for(auto& c:manualControls){bool visible=c.page==page;c.label->setVisible(visible);
        if(c.knob)c.knob->setVisible(visible);
        if(c.choice)c.choice->setVisible(visible);
        if(c.toggle)c.toggle->setVisible(visible);
        if(!visible)continue;
        int column=page==2?index/3:index%columns,row=page==2?index%3:index/columns;
        int x=20+column*cellW,y=104+row*cellH;++index;
        c.label->setBounds(x,y,cellW,24);
        if(c.knob)c.knob->setBounds(x+8,y+26,cellW-16,juce::jmin(150,cellH-32));
        if(c.choice)c.choice->setBounds(x+10,y+42,cellW-20,34);
        if(c.toggle)c.toggle->setBounds(x+20,y+42,cellW-40,34);
    }
}
void MataleTuneAudioProcessorEditor::timerCallback(){int state=processor.autoState.load(std::memory_order_acquire),last=processor.lastAutoStatus.load();autoButton.setEnabled(state!=1&&state!=2);autoButton.setButtonText(state==1?"LISTENING":"AUTO");
    juce::String message="Matale Tune 0.9.0 DAW preview | Play a vocal and press AUTO.";
    if(state==1)message="Listening: "+juce::String(int(processor.progress.load()*100))+"% | Keep the DAW playing a voiced phrase.";else if(last==3)message="No stable vocal found. Play a voiced phrase and press AUTO again.";else if(last==2)message="Auto applied | Key uncertain: retry on a phrase with several different notes. Chromatic pitch retained.";else if(last==1){int root=processor.detectedRoot.load();const char* names[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};message="Auto applied | EQ, dynamics, smoothing and included effects ready.";if(root>=0)message+=" Key: "+juce::String(names[root])+ (processor.detectedScale.load()==1?" Major":" Minor");}
    const bool opto=processor.parameters.getRawParameterValue("optoMode")->load()>.5f;
    for(auto& c:manualControls)if(c.knob){auto id=c.knob->getComponentID();
        if(id=="ratio"||id=="attack"||id=="release"){
            c.knob->setEnabled(!opto);c.label->setEnabled(!opto);
            c.knob->setTooltip("Standard compressor only. Disable Opto compressor to adjust ratio, attack and release.");
        }
    }
    status.setText(message,juce::dontSendNotification);
    int write=processor.scopePosition.load(std::memory_order_acquire);fftData.fill(0);for(int i=0;i<2048;++i)fftData[size_t(i)]=processor.scope[size_t((write+i)%2048)].load(std::memory_order_relaxed)*(.5f-.5f*std::cos(2*matale::pi*i/2047.f));fft.performFrequencyOnlyForwardTransform(fftData.data());double rate=processor.getSampleRate()>0?processor.getSampleRate():48000;for(int i=0;i<int(spectrum.size());++i){float frequency=20*std::pow(1000.f,float(i)/float(spectrum.size()-1));int bin=juce::jlimit(1,1023,int(frequency/rate*2048));float level=matale::db(fftData[size_t(bin)]/512.f);spectrum[size_t(i)]=.65f*spectrum[size_t(i)]+.35f*juce::jlimit(0.f,1.f,(level+80)/80);}
    repaint();
}
void MataleTuneAudioProcessorEditor::paint(juce::Graphics& g){g.fillAll(juce::Colour(0xfff3f8fd));if(manualOpen){card(g,manualPanel.getBounds());text(g,"Manual controls",{32,24,400,30},22,ink,true);text(g,"Live parameters / double-click a knob to reset / type a value below each knob",{350,64,getWidth()-390,30},12,muted);return;}for(auto r:{analysisBounds,outputBounds,autoBounds,smoothBounds})card(g,r);
    auto a=analysisBounds;float hz=processor.displayedPitch.load(),confidence=processor.pitchConfidence.load();int midi=hz>0?juce::roundToInt(69+12*std::log2(hz/440.f)):-1;
    text(g,"Vocal Analysis",{a.getX()+20,a.getY()+16,220,26},19,ink,true);text(g,hz>0?"Live / voiced":"No voiced input",{a.getRight()-130,a.getY()+18,112,22},11,hz>0?green:muted,false,juce::Justification::centredRight);
    auto ring=juce::Rectangle<float>(100,100).withCentre({float(a.getX()+73),float(a.getCentreY()+10)});g.setColour(juce::Colour(0xffcfedf9));g.drawEllipse(ring,8);juce::Path arc;arc.addCentredArc(ring.getCentreX(),ring.getCentreY(),50,50,0,-2.5f,-2.5f+5*juce::jlimit(0.f,1.f,confidence),true);g.setColour(blue);g.strokePath(arc,juce::PathStrokeType(8));text(g,noteName(midi),{a.getX()+23,a.getCentreY()-10,100,36},28,ink,true,juce::Justification::centred);text(g,hz>0?juce::String(hz,1)+" Hz":"No pitch",{a.getX()+23,a.getCentreY()+29,100,20},11,muted,false,juce::Justification::centred);
    int sx=a.getX()+146,sw=juce::jlimit(140,180,a.getWidth()/4),sy=a.getY()+70;int min=processor.minNote.load(),max=processor.maxNote.load(),median=processor.medianNote.load();juce::String reg=median<0?"More notes":median<48?"Bass":median<55?"Baritone":median<60?"Tenor":median<65?"Alto":median<69?"Mezzo":"Soprano";juce::String titles[]={"Detected note","Confidence","Vocal range","Register (est.)","Energy / RMS"};juce::String vals[]={noteName(midi),hz>0?juce::String(int(confidence*100))+"%":"--",min<=max?noteName(min)+" - "+noteName(max):"--",reg,juce::String(processor.inputRms.load()*100,1)+"% FS"};for(int i=0;i<5;++i){text(g,titles[i],{sx,sy+i*25,sw-67,20},11,muted);text(g,vals[i],{sx+sw-85,sy+i*25,85,20},11,ink,false,juce::Justification::centredRight);}
    auto plot=juce::Rectangle<int>(sx+sw+16,a.getY()+74,a.getRight()-sx-sw-36,a.getHeight()-128);text(g,"LIVE OUTPUT SPECTRUM",{plot.getX(),plot.getY()-22,plot.getWidth(),18},9,muted);g.setColour(line);g.drawHorizontalLine(plot.getBottom(),float(plot.getX()),float(plot.getRight()));juce::Path path;for(int i=0;i<int(spectrum.size());++i){float x=plot.getX()+plot.getWidth()*float(i)/float(spectrum.size()-1),y=plot.getBottom()-spectrum[size_t(i)]*plot.getHeight();if(i==0)path.startNewSubPath(x,y);else path.lineTo(x,y);}g.setColour(blue);g.strokePath(path,juce::PathStrokeType(1.5f));meter(g,{float(plot.getX()),float(a.getBottom()-32),float(juce::jmax(20,plot.getWidth()-80)),7},processor.inputLevel.load());text(g,decibel(processor.inputLevel.load())+" dBFS",{plot.getRight()-80,a.getBottom()-41,80,24},10,ink,false,juce::Justification::centredRight);
    auto o=outputBounds;text(g,"Output",{o.getX()+20,o.getY()+16,160,26},19,ink,true);text(g,decibel(processor.outputLevel.load())+" dBFS",{o.getX()+20,o.getY()+62,o.getWidth()-40,44},29,ink,true);meter(g,{float(o.getX()+20),float(o.getY()+121),float(o.getWidth()-40),11},processor.outputLevel.load());float lufs=processor.momentaryLufs.load();text(g,"Momentary LUFS",{o.getX()+20,o.getBottom()-73,130,20},10,muted);text(g,lufs>-179?juce::String(lufs,1):"--",{o.getX()+20,o.getBottom()-49,100,24},16,ink,true);text(g,"Sample peak",{o.getX()+150,o.getBottom()-73,110,20},10,muted);text(g,decibel(processor.samplePeak.load())+" dBFS",{o.getX()+150,o.getBottom()-27,110,20},12,ink);
    for(int c=0;c<2;++c){float level=c?processor.rightLevel.load():processor.leftLevel.load();int x=o.getRight()-42+c*18,y=o.getY()+151,h=juce::jmax(25,o.getHeight()-175);g.setColour(line);g.fillRoundedRectangle(float(x),float(y),8,float(h),4);float fill=juce::jlimit(0.f,1.f,(matale::db(level)+60)/60);g.setColour(c?cyan:green);g.fillRoundedRectangle(float(x),float(y+h)-h*fill,8,h*fill,4);text(g,c?"R":"L",{x-2,y-18,14,16},9,muted);}
    auto b=autoBounds;text(g,"Auto Mode",{b.getX()+20,b.getY()+16,220,28},20,ink,true);text(g,"Play a vocal. Press AUTO. Fine-tune if needed.",{b.getX()+20,b.getY()+44,b.getWidth()-40,20},11,muted);int state=processor.autoState.load();if(state==1){meter(g,{float(b.getX()+20),float(b.getBottom()-30),float(b.getWidth()-40),8},matale::gain(processor.progress.load()*60-60));}else text(g,"Dry analysis | adaptive EQ / compression | natural pitch",{b.getX()+20,b.getBottom()-32,b.getWidth()-40,22},11,muted);
    auto s=smoothBounds;text(g,"Vocal Smooth",{s.getX()+20,s.getY()+16,s.getWidth()-40,28},20,ink,true);text(g,"Dynamic harshness reduction and tonal shaping.",{s.getX()+20,s.getY()+51,s.getWidth()-40,22},11,muted);auto result=juce::Rectangle<int>(s.getX()+18,s.getBottom()-102,s.getWidth()-36,77);g.setColour(juce::Colour(0xffeaf7ff));g.fillRoundedRectangle(result.toFloat(),10);text(g,"Measured processing",result.reduced(12).withHeight(22),12,blue,true);text(g,"Smooth "+juce::String(processor.smoothReduction.load(),1)+" dB  /  De-Esser "+juce::String(processor.deessReduction.load(),1)+" dB",{result.getX()+12,result.getY()+38,result.getWidth()-24,24},11,muted);
}
