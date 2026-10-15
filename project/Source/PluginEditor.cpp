#include "PluginEditor.h"
namespace {
#if JUCE_WINDOWS
const juce::String uiFont="Segoe UI";
#else
const juce::String uiFont="DejaVu Sans";
#endif
const juce::Colour ink(0xff192238), muted(0xff6e7a91), blue(0xff245ef4), cyan(0xff73a7ff);
const juce::Colour line(0xffd3e0ec), green(0xff13cda0);
void text(juce::Graphics& g, const juce::String& s, juce::Rectangle<int> bounds,
          float size = 14.f, juce::Colour colour = ink, bool bold = false,
          juce::Justification justify = juce::Justification::centredLeft) {
    g.setColour(colour); g.setFont(juce::FontOptions(uiFont,size, bold ? juce::Font::bold : juce::Font::plain));
    g.drawText(s, bounds, justify, true);
}
void card(juce::Graphics& g, juce::Rectangle<int> bounds) {
    const float radius=bounds.getWidth()>650?24.f:19.f;
    juce::DropShadow(juce::Colour(0x163b5c82),16,{0,5}).drawForRectangle(g,bounds.reduced(5));
    g.setColour(juce::Colours::white);g.fillRoundedRectangle(bounds.toFloat(),radius);
    g.setColour(juce::Colour(0xffd7e3ee));g.drawRoundedRectangle(bounds.toFloat().reduced(.5f),radius,1.f);
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
    setDefaultSansSerifTypefaceName(uiFont);
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
    setColour(juce::TextButton::textColourOffId, ink);
    setColour(juce::TextButton::textColourOnId, blue);
}
void MataleLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& b,
                                             const juce::Colour&, bool over, bool down) {
    auto r = b.getLocalBounds().toFloat().reduced(1);
    if (b.getComponentID() == "auto-orb") {
        r=r.withSizeKeepingCentre(juce::jmin(r.getWidth(),r.getHeight()),juce::jmin(r.getWidth(),r.getHeight())).reduced(5);
        juce::Path disc;disc.addEllipse(r.reduced(10));juce::DropShadow(juce::Colour(0x203a5f8a),12,{0,5}).drawForPath(g,disc);
        g.setColour(juce::Colours::white);g.fillEllipse(r.reduced(10));
        float radius=r.getWidth()*.5f-3;auto centre=r.getCentre();
        juce::Path track,arc;track.addCentredArc(centre.x,centre.y,radius,radius,0,-2.4f,2.4f,true);
        float amount=b.isEnabled()?.78f:juce::jlimit(.03f,1.f,float(b.getProperties().getWithDefault("progress",0.f)));
        arc.addCentredArc(centre.x,centre.y,radius,radius,0,-2.4f,-2.4f+4.8f*amount,true);
        g.setColour(line);g.strokePath(track,juce::PathStrokeType(4));g.setColour(blue);g.strokePath(arc,juce::PathStrokeType(4.5f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    }else{
        const bool active=b.getComponentID()=="manual"||b.getToggleState();
        g.setColour(active?(down?blue.darker(.15f):blue):over?juce::Colour(0xffedf3ff):juce::Colours::white);
        g.fillRoundedRectangle(r,r.getHeight()*.46f);
        if(!active){g.setColour(line);g.drawRoundedRectangle(r,r.getHeight()*.46f,1);}
    }
}
void MataleLookAndFeel::drawButtonText(juce::Graphics& g, juce::TextButton& b, bool over, bool down) {
    if(b.getComponentID()=="manual"||b.getToggleState()){
        text(g,b.getButtonText(),b.getLocalBounds(),13.f,juce::Colours::white,true,juce::Justification::centred);return;
    }
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
    g.setColour(blue.withAlpha(b.isEnabled()?1.f:.6f)); g.fillPath(sparkle);
    text(g,b.getButtonText(),{0,cy-14,r.getWidth(),38},b.isEnabled()?29.f:19.f,blue,true,juce::Justification::centred);
    text(g,"3 sec vocal analysis",{0,cy+28,r.getWidth(),22},11.f,muted,false,juce::Justification::centred);
}
void MataleLookAndFeel::drawComboBox(juce::Graphics& g,int w,int h,bool,int,int,int,int,juce::ComboBox& box){
    auto r=juce::Rectangle<float>(.5f,.5f,float(w)-1,float(h)-1);g.setColour(box.isEnabled()?juce::Colours::white:juce::Colour(0xfff2f6fa));g.fillRoundedRectangle(r,11);
    g.setColour(line);g.drawRoundedRectangle(r,11,1);juce::Path arrow;arrow.startNewSubPath(float(w-23),h*.44f);arrow.lineTo(float(w-18),h*.59f);arrow.lineTo(float(w-13),h*.44f);
    g.setColour(muted);g.strokePath(arrow,juce::PathStrokeType(1.5f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
}
void MataleLookAndFeel::drawToggleButton(juce::Graphics& g,juce::ToggleButton& button,bool over,bool){
    auto r=button.getLocalBounds();auto box=juce::Rectangle<float>(16,16).withCentre({12.f,float(r.getCentreY())});
    g.setColour(button.getToggleState()?blue:juce::Colour(0xffe9f1f9));g.fillRoundedRectangle(box,5);
    if(button.getToggleState()){
        juce::Path tick;tick.startNewSubPath(box.getX()+4,box.getY()+8);tick.lineTo(box.getX()+7,box.getY()+11);tick.lineTo(box.getX()+12,box.getY()+5);
        g.setColour(juce::Colours::white);g.strokePath(tick,juce::PathStrokeType(1.8f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    }else {g.setColour(over?blue:line);g.drawRoundedRectangle(box,5,1);}
    text(g,button.getButtonText(),r.withTrimmedLeft(28),12.f,button.isEnabled()?ink:muted);
}
void MataleLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int w, int h,
                                         float position, float start, float end, juce::Slider&) {
    auto r = juce::Rectangle<float>(float(x), float(y), float(w), float(h)).reduced(9);
    float radius = juce::jmin(r.getWidth(), r.getHeight()) * .5f;
    auto centre = r.getCentre();
    juce::Path track, value;
    track.addCentredArc(centre.x, centre.y, radius, radius, 0, start, end, true);
    value.addCentredArc(centre.x, centre.y, radius, radius, 0, start, start + position * (end-start), true);
    g.setColour(line); g.strokePath(track, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    g.setGradientFill(juce::ColourGradient(cyan,r.getX(),r.getY(),blue,r.getRight(),r.getBottom(),false)); g.strokePath(value, juce::PathStrokeType(3.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    auto disc = juce::Rectangle<float>(radius*1.65f, radius*1.65f).withCentre(centre);
    juce::DropShadow(juce::Colour(0x253b6a98),6,{0,3}).drawForPath(g,[&]{juce::Path p;p.addEllipse(disc);return p;}());
    g.setGradientFill(juce::ColourGradient(juce::Colours::white, disc.getX(), disc.getY(),
                       juce::Colour(0xfff1f6fc), disc.getRight(), disc.getBottom(), false));
    g.fillEllipse(disc);g.setColour(juce::Colours::white);g.drawEllipse(disc.reduced(1),1.5f);
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
    peakReset.onClick=[this]{processor.requestPeakReset.store(true);};status.setFont(juce::FontOptions(12.f));status.setColour(juce::Label::textColourId,muted);status.setText("Matale Tune 0.13.0 DAW preview | Play a vocal and press AUTO.",juce::dontSendNotification);
    addAndMakeVisible(manualButton);manualButton.setComponentID("manual");
    manualButton.onClick=[this]{showManual(!manualOpen);};
    addChildComponent(manualPanel);manualPanel.addAndMakeVisible(manualPage);
    manualPanel.addAndMakeVisible(moduleDefault);moduleDefault.setComponentID("module-default");
    moduleDefault.onClick=[this]{noticeProfile=-1;presetNotice=processor.resetDefaults(manualPage.getSelectedId()-1)?"This module restored to Default.":"Wait for Auto analysis to finish before restoring Default.";noticeUntil=juce::Time::getMillisecondCounterHiRes()+5000;};
    manualPanel.addAndMakeVisible(modulePreset);modulePreset.setComponentID("module-preset");
    modulePreset.onChange=[this]{int index=modulePreset.getSelectedId()-2;if(index<0)return;
        noticeProfile=-1;
        const char* names[]={"Pitch","Compressor","EQ","Space","Vocal cleanup"};int page=manualPage.getSelectedId()-1;
        presetNotice=processor.applyModulePreset(page,index)?juce::String(names[juce::jlimit(0,4,page)])+": "+modulePreset.getText()+" applied.":"Wait for Auto analysis to finish before changing module preset.";
        noticeUntil=juce::Time::getMillisecondCounterHiRes()+5000;
    };
    manualPage.setComponentID("manual-page");
    manualPage.addItemList({"Pitch / Key", "Compressor", "EQ", "Delay / Reverb / Output", "Vocal / Denoise"},1);
    manualPage.setSelectedId(1);manualPage.onChange=[this]{layoutManual();repaint();};
    manualPage.setVisible(false);
    const char* tabNames[]={"Pitch / Key","Compressor","EQ","Space / Output","Vocal cleanup"};
    for(int i=0;i<5;++i){auto& tab=pageTabs[size_t(i)];manualPanel.addAndMakeVisible(tab);tab.setButtonText(tabNames[i]);tab.setComponentID("page-tab");tab.onClick=[this,i]{manualPage.setSelectedId(i+1,juce::sendNotificationSync);};}
    addAndMakeVisible(factoryPreset);factoryPreset.setComponentID("factory-preset");
    factoryPreset.addItemList({"Natural","Clear","Warm","Bright","Hard Tune"},1);
    factoryPreset.setSelectedId(1);factoryPreset.onChange=[this]{
        noticeProfile=-1;
        if(!processor.applyFactoryPreset(factoryPreset.getSelectedId()-1)){presetNotice="Wait for Auto analysis to finish before changing character.";}
        else {noticeProfile=factoryPreset.getSelectedId()-1;presetNotice=factoryPreset.getText()+" preset applied. Press AUTO to adapt it to your vocal.";}
        noticeUntil=juce::Time::getMillisecondCounterHiRes()+6000;
    };
    addAndMakeVisible(defaultButton);defaultButton.setComponentID("global-default");
    defaultButton.onClick=[this]{noticeProfile=-1;presetNotice=processor.resetDefaults()?"All settings restored to Default.":"Wait for Auto analysis to finish before restoring Default.";noticeUntil=juce::Time::getMillisecondCounterHiRes()+5000;};
    addAndMakeVisible(saveButton);addAndMakeVisible(loadButton);
    saveButton.onClick=[this]{choosePreset(true);};loadButton.onClick=[this]{choosePreset(false);};
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
            control.knob->setColour(juce::Slider::textBoxOutlineColourId,juce::Colours::transparentBlack);
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
        juce::String label=d.name;
        if(i==2)label="Tuning mode";
        if(i>=13&&i<=27){const char* eqLabels[]={"Frequency (Hz)","Gain (dB)","Q / Width"};label=eqLabels[(i-13)%3];}
        addControl(d.id,label.toRawUTF8(),d.initial,d.page,false,options);
    }
    for(int i=0;i<=matale::Safety;++i){const auto& d=matale::nativeParameters[i];
        int page=(i==matale::Opto||i==matale::OptoMix||i==matale::SidechainHz||i==matale::Limit)?1: i==matale::Safety?3:4;
        addControl(d.id,d.name,d.initial,page,d.boolean);
    }
    addControl("pitchResponse","Style",0,0,false,{"Neutral","Soft","Tight","Pop","Rap","Ballad"});
    setResizable(true,true);setResizeLimits(980,700,1800,1000);addAndMakeVisible(credit);credit.setText("Make by Myominaung",juce::dontSendNotification);
    credit.setFont(juce::FontOptions(12.5f));credit.setColour(juce::Label::textColourId,ink);
    addAndMakeVisible(telegram);telegram.setFont(juce::FontOptions(12.5f),false,juce::Justification::centredRight);
    telegram.setColour(juce::HyperlinkButton::textColourId,blue);
    setSize(1200,760);startTimerHz(24);
}
MataleTuneAudioProcessorEditor::~MataleTuneAudioProcessorEditor(){stopTimer();setLookAndFeel(nullptr);}
void MataleTuneAudioProcessorEditor::resized(){
    const int pad=16,gap=16,w=getWidth()-pad*2,left=int((w-gap)*.64f),y=86;
    const int available=getHeight()-y-76,top=juce::jlimit(210,280,int(available*.42f));
    analysisBounds={pad,y,left,top};outputBounds={pad+left+gap,y,w-left-gap,top};
    autoBounds={pad,y+top+gap,left,available-top-gap};
    smoothBounds={outputBounds.getX(),autoBounds.getY(),outputBounds.getWidth(),autoBounds.getHeight()};
    manualButton.setBounds(getWidth()-184,24,164,36);
    defaultButton.setBounds(getWidth()-266,28,74,32);
    factoryPreset.setBounds(getWidth()-650,28,210,32);
    saveButton.setBounds(getWidth()-430,28,74,32);loadButton.setBounds(getWidth()-348,28,74,32);
    manualPanel.setBounds(pad,y,w,available);layoutManual();
    status.setBounds(pad+2,getHeight()-62,w-4,24);
    credit.setBounds(pad,getHeight()-30,240,22);telegram.setBounds(getWidth()-220,getHeight()-30,200,22);
    power.setBounds(outputBounds.getRight()-82,outputBounds.getY()+16,68,25);
    peakReset.setBounds(outputBounds.getX()+outputBounds.getWidth()/2-30,outputBounds.getBottom()-54,94,24);
    const int row=juce::jmin(26,(autoBounds.getHeight()-98)/7);
    autoKey.setBounds(autoBounds.getX()+20,autoBounds.getY()+62,juce::jmin(310,autoBounds.getWidth()-210),23);
    for(int i=0;i<7;++i)includes[size_t(i)].setBounds(autoBounds.getX()+20,autoBounds.getY()+90+i*row,juce::jmin(290,autoBounds.getWidth()-210),row);
    int orb=juce::jlimit(172,248,juce::jmin(autoBounds.getWidth()/3,autoBounds.getHeight()-42));
    autoButton.setBounds(autoBounds.getRight()-orb-20,autoBounds.getCentreY()-orb/2+6,orb,orb);
    const int cell=(smoothBounds.getWidth()-32)/3,knobY=smoothBounds.getY()+94;
    const int h=juce::jlimit(52,112,smoothBounds.getHeight()-202);
    for(int i=0;i<3;++i){int x=smoothBounds.getX()+16+i*cell;
        knobLabels[size_t(i)].setBounds(x,knobY-25,cell,24);knobs[size_t(i)].setBounds(x,knobY,cell,h);}
    harsh.setBounds(smoothBounds.getRight()-136,smoothBounds.getY()+18,122,25);
}
void MataleTuneAudioProcessorEditor::showManual(bool open){
    manualOpen=open;
    for(auto* child:getChildren())if(child!=&manualPanel&&child!=&manualButton&&child!=&status&&child!=&credit&&child!=&telegram&&child!=&factoryPreset&&child!=&saveButton&&child!=&loadButton&&child!=&defaultButton)child->setVisible(!open);
    manualPanel.setVisible(open);manualButton.setButtonText(open?"Back to Auto":"Manual controls");
    layoutManual();repaint();
}
void MataleTuneAudioProcessorEditor::choosePreset(bool saving){
    presetChooser=std::make_unique<juce::FileChooser>(saving?"Save Matale Tune preset":"Load Matale Tune preset",juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("My Vocal.matalepreset"),"*.matalepreset");
    auto safe=juce::Component::SafePointer<MataleTuneAudioProcessorEditor>(this);
    int flags=saving?(juce::FileBrowserComponent::saveMode|juce::FileBrowserComponent::canSelectFiles|juce::FileBrowserComponent::warnAboutOverwriting):(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles);
    presetChooser->launchAsync(flags,[safe,saving](const juce::FileChooser& chooser){
        if(!safe)return;auto file=chooser.getResult();if(file==juce::File{})return;
        if(saving){auto target=file.withFileExtension("matalepreset");
            if(target!=file&&target.existsAsFile()){safe->presetNotice="That preset already exists. Choose its .matalepreset filename to replace it.";safe->noticeUntil=juce::Time::getMillisecondCounterHiRes()+7000;return;}
            file=target;
        }
        juce::String error;bool ok=saving?safe->processor.savePreset(file,error):safe->processor.loadPreset(file,error);
        safe->noticeProfile=-1;
        safe->presetNotice=ok?(saving?"Saved: ":"Loaded: ")+file.getFileName():error;
        safe->noticeUntil=juce::Time::getMillisecondCounterHiRes()+7000;
    });
}
void MataleTuneAudioProcessorEditor::layoutManual(){
    const int page=manualPage.getSelectedId()-1,startX=184,width=manualPanel.getWidth()-204,height=manualPanel.getHeight();
    manualModules.clear();manualPage.setBounds(0,0,1,1);manualPage.setVisible(false);
    moduleDefault.setBounds(manualPanel.getWidth()-(page>0?342:102),12,80,32);
    modulePreset.setBounds(manualPanel.getWidth()-252,12,232,32);modulePreset.setVisible(page>0);
    modulePreset.clear(juce::dontSendNotification);modulePreset.addItem("Custom",1);modulePreset.setItemEnabled(1,false);
    auto moduleNames=processor.modulePresetNames(page);for(int i=0;i<moduleNames.size();++i)modulePreset.addItem(moduleNames[i],i+2);
    modulePreset.setSelectedId(processor.modulePresetIndex(page)+2,juce::dontSendNotification);
    for(int i=0;i<5;++i){pageTabs[size_t(i)].setBounds(16,76+i*46,150,38);pageTabs[size_t(i)].setToggleState(page==i,juce::dontSendNotification);}
    if(page==0){manualModules.push_back({startX,64,width,118});manualModules.push_back({startX,198,width,juce::jmin(240,height-216)});}
    else if(page==2){for(int b=0;b<5;++b)manualModules.push_back({startX+b*(width/5),174,width/5-8,height-192});}
    else if(page==1){manualModules.push_back({startX,64,width,58});for(int r=0;r<2;++r)manualModules.push_back({startX,138+r*((height-154)/2),width,(height-154)/2-10});}
    else {for(int i=0;i<3;++i)manualModules.push_back({startX+i*(width/3),64,width/3-10,height-82});}
    int index=0,pitchChoice=0,pitchKnob=0;
    for(auto& c:manualControls){bool visible=c.page==page;c.label->setVisible(visible);
        if(c.knob)c.knob->setVisible(visible);
        if(c.choice)c.choice->setVisible(visible);
        if(c.toggle)c.toggle->setVisible(visible);
        if(!visible)continue;
        juce::String id=c.knob?c.knob->getComponentID():c.choice?c.choice->getComponentID():c.toggle->getComponentID();
        int x=startX,y=94,w=width,h=128;
        if(page==0){if(c.choice){w=width/4;x+=pitchChoice++*w;y=94;h=72;}else {int k=pitchKnob++;w=width/4;x+=k*w;y=232;h=juce::jmin(156,height-250);}}
        else if(page==1){if(c.toggle){int col=id=="optoMode"?0:1;w=width/2;x+=col*w;y=76;h=36;}
            else{int k=0;const char* ids[]={"threshold","ratio","attack","release","makeup","optoMix","sidechainHz"};while(k<7&&id!=ids[k])++k;w=width/4;x+=(k%4)*w;h=juce::jmin(155,(height-170)/2);y=154+(k/4)*((height-154)/2);}}
        else if(page==2){int k=index/3,row=index%3;w=width/5;x+=k*w;int rowH=(height-234)/3;y=215+row*rowH;h=rowH-4;}
        else {int group=0,row=0;
            if(page==3){if(id=="delayMix")row=0;else if(id=="delayBeats")row=1;else if(id=="feedback")row=2;else if(id=="reverbMix"){group=1;row=0;}else if(id=="room"){group=1;row=1;}else{group=2;row=id=="output"?0:id=="bpm"?1:2;}}
            else {if(id=="vocalSmooth")row=0;else if(id=="vocalWarmth")row=1;else if(id=="vocalPresence")row=2;else if(id=="harshControl")row=3;else if(id=="deessAmount"){group=1;row=0;}else if(id=="deessHz"){group=1;row=1;}else if(id=="deessThreshold"){group=1;row=2;}else{group=2;row=id=="denoiseAmount"?0:1;}}
            w=width/3;x+=group*w;int rowH=juce::jmin(126,(height-130)/3);y=106+row*rowH;h=c.toggle?34:rowH-4;
        }
        ++index;
        if(c.toggle)y=juce::jmin(y,height-40);
        if(c.toggle){c.label->setBounds(x+12,y,w/2,28);c.toggle->setBounds(x+w/2,y,w/2-14,28);}
        else {c.label->setBounds(x+6,y,w-18,22);
            if(c.choice)c.choice->setBounds(x+14,y+28,w-32,32);
            if(c.knob)c.knob->setBounds(x+12,y+23,w-28,juce::jmax(30,h-26));}
    }
}
void MataleTuneAudioProcessorEditor::paintManual(juce::Graphics& g){
    auto panel=manualPanel.getBounds();card(g,panel);juce::Graphics::ScopedSaveState save(g);g.setOrigin(panel.getPosition());
    const int page=manualPage.getSelectedId()-1;const char* names[]={"Pitch / Key","Compressor","Equalizer","Space / Output","Vocal cleanup"};
    text(g,names[juce::jlimit(0,4,page)],{184,14,400,30},22,ink,true);
    text(g,"VOCAL TOOLS",{20,28,145,24},11,muted,true);
    for(auto r:manualModules)card(g,r);
    auto live=juce::Rectangle<int>(16,330,150,juce::jmin(146,panel.getHeight()-346));card(g,live);
    text(g,"LIVE INPUT",{28,340,126,20},10,blue,true);
    float hz=processor.displayedPitch.load();int midi=hz>0?juce::roundToInt(69+12*std::log2(hz/440.f)):-1;
    text(g,noteName(midi),{28,361,126,32},23,ink,true);
    meter(g,{28.f,402.f,126.f,6.f},processor.inputLevel.load());
    text(g,"Out "+decibel(processor.outputLevel.load())+" dBFS",{28,416,126,18},11,muted);
    const char* tips[]={"Styles shape Natural mode around your response knobs; Hard Tune keeps fixed fast timing.","Opto: smooth program-dependent timing. Switch off for standard controls.","Five parametric bands / frequency, gain and Q.","Tempo-synced delay, room ambience and output safety.","Tone shaping, sibilance control and quiet-noise reduction."};
    text(g,tips[juce::jlimit(0,4,page)],{184,44,panel.getWidth()-206,18},11,muted);
    if(page==0){text(g,"MUSICAL TARGET",{198,72,260,20},10,blue,true);text(g,"PITCH RESPONSE",{198,205,260,20},10,blue,true);}
    if(page==2){
        auto plot=juce::Rectangle<int>(184,70,panel.getWidth()-204,86);
        g.setColour(juce::Colour(0xffeaf4ff));g.fillRoundedRectangle(plot.toFloat(),10);
        std::array<matale::Biquad,5> bands;double sr=processor.getSampleRate()>0?processor.getSampleRate():48000;
        for(int b=0;b<5;++b){int i=13+b*3;bands[size_t(b)].peak(sr,processor.parameters.getRawParameterValue(matale::parameterList[i].id)->load(),processor.parameters.getRawParameterValue(matale::parameterList[i+2].id)->load(),processor.parameters.getRawParameterValue(matale::parameterList[i+1].id)->load());}
        g.setColour(line);g.drawHorizontalLine(plot.getCentreY(),float(plot.getX()),float(plot.getRight()));juce::Path curve;
        for(int i=0;i<240;++i){double hz=20*std::pow(1000.,i/239.),omega=2*matale::pi*std::min(hz,sr*.49)/sr,magnitude=1;
            for(auto& b:bands){double nr=b.b0+b.b1*std::cos(omega)+b.b2*std::cos(2*omega),ni=-b.b1*std::sin(omega)-b.b2*std::sin(2*omega),dr=1+b.a1*std::cos(omega)+b.a2*std::cos(2*omega),di=-b.a1*std::sin(omega)-b.a2*std::sin(2*omega);magnitude*=std::sqrt((nr*nr+ni*ni)/std::max(1.e-16,dr*dr+di*di));}
            float x=plot.getX()+i*plot.getWidth()/239.f,y=plot.getCentreY()-juce::jlimit(-18.f,18.f,float(20*std::log10(std::max(1.e-9,magnitude))))*plot.getHeight()/40.f;
            if(i==0)curve.startNewSubPath(x,y);else curve.lineTo(x,y);
        }
        g.setColour(blue);g.strokePath(curve,juce::PathStrokeType(2));
        for(int b=0;b<5;++b){const char* names[]={"LOW","LOW MID","MID","HIGH MID","HIGH"};auto r=manualModules[size_t(b)];
            const juce::Colour accents[]={juce::Colour(0xff2675db),juce::Colour(0xff15a28c),juce::Colour(0xff7561ce),juce::Colour(0xffce8a28),juce::Colour(0xffce5a75)};
            g.setColour(accents[b]);g.fillRoundedRectangle(float(r.getX()+12),float(r.getY()+10),float(r.getWidth()-24),3.f,1.5f);
            text(g,names[b],r.reduced(12).translated(0,7).withHeight(20),12,ink,true);
        }
    }
    if(page>=3){const char* space[]={"DELAY","REVERB","OUTPUT"};const char* cleanup[]={"TONE / SMOOTH","DE-ESSER","DENOISE"};for(int i=0;i<3;++i)text(g,page==3?space[i]:cleanup[i],manualModules[size_t(i)].reduced(12).withHeight(24),11,blue,true);}
}
void MataleTuneAudioProcessorEditor::timerCallback(){int state=processor.autoState.load(std::memory_order_acquire),last=processor.lastAutoStatus.load();autoButton.setEnabled(state!=1&&state!=2);autoButton.setButtonText(state==1?"LISTENING":"AUTO");autoButton.getProperties().set("progress",processor.progress.load());
    juce::String message="Matale Tune 0.13.0 DAW preview | Play a vocal and press AUTO.";
    if(state==1)message="Listening: "+juce::String(int(processor.progress.load()*100))+"% | Keep the DAW playing a voiced phrase.";else if(last==3)message="No stable vocal found. Play a voiced phrase and press AUTO again.";else if(last==2)message="Auto applied | Key uncertain: retry on a phrase with several different notes. Chromatic pitch retained.";else if(last==1){int root=processor.detectedRoot.load();const char* names[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};message="Auto applied | EQ, dynamics, smoothing and included effects ready.";if(root>=0)message+=" Key: "+juce::String(names[root])+ (processor.detectedScale.load()==1?" Major":" Minor");}
    const bool opto=processor.parameters.getRawParameterValue("optoMode")->load()>.5f;
    for(auto& c:manualControls)if(c.knob){auto id=c.knob->getComponentID();
        if(id=="ratio"||id=="attack"||id=="release"){
            c.knob->setEnabled(!opto);c.label->setEnabled(!opto);
            c.knob->setTooltip("Standard compressor only. Disable Opto compressor to adjust ratio, attack and release.");
        }
    }
    if(manualOpen)message="Manual / Live changes are saved with your DAW project. Auto updates only the groups you include.";
    const bool natural=processor.parameters.getRawParameterValue("style")->load()<.5f;
    for(auto& c:manualControls)if(c.choice&&c.choice->getComponentID()=="pitchResponse"){c.choice->setEnabled(natural);c.label->setEnabled(natural);}
    factoryPreset.setSelectedId(int(processor.parameters.getRawParameterValue("autoProfile")->load())+1,juce::dontSendNotification);
    if(manualOpen&&manualPage.getSelectedId()>1)modulePreset.setSelectedId(processor.modulePresetIndex(manualPage.getSelectedId()-1)+2,juce::dontSendNotification);
    if(noticeProfile>=0&&noticeProfile!=int(processor.parameters.getRawParameterValue("autoProfile")->load()))noticeUntil=0;
    if(juce::Time::getMillisecondCounterHiRes()<noticeUntil)message=presetNotice;
    status.setText(message,juce::dontSendNotification);
    int write=processor.scopePosition.load(std::memory_order_acquire);fftData.fill(0);for(int i=0;i<2048;++i)fftData[size_t(i)]=processor.scope[size_t((write+i)%2048)].load(std::memory_order_relaxed)*(.5f-.5f*std::cos(2*matale::pi*i/2047.f));fft.performFrequencyOnlyForwardTransform(fftData.data());double rate=processor.getSampleRate()>0?processor.getSampleRate():48000;for(int i=0;i<int(spectrum.size());++i){float frequency=20*std::pow(1000.f,float(i)/float(spectrum.size()-1));int bin=juce::jlimit(1,1023,int(frequency/rate*2048));float level=matale::db(fftData[size_t(bin)]/512.f);spectrum[size_t(i)]=.65f*spectrum[size_t(i)]+.35f*juce::jlimit(0.f,1.f,(level+80)/80);}
    repaint();
}
void MataleTuneAudioProcessorEditor::paint(juce::Graphics& g){
    g.setGradientFill(juce::ColourGradient(juce::Colour(0xffeaf3f9),0,0,juce::Colour(0xffe3edf7),float(getWidth()),float(getHeight()),false));g.fillAll();
    g.setColour(juce::Colours::white.withAlpha(.82f));g.fillRect(0,0,getWidth(),74);
    g.setColour(line);g.drawHorizontalLine(74,0.f,float(getWidth()));
    auto logo=juce::Rectangle<float>(20,17,48,46);
    g.setColour(juce::Colour(0xffe8f6ff));g.fillRoundedRectangle(logo,13);
    juce::Path mark;mark.startNewSubPath(29,52);mark.lineTo(36,28);mark.lineTo(44,43);mark.lineTo(54,28);mark.lineTo(60,52);
    g.setGradientFill(juce::ColourGradient(cyan,29,28,juce::Colour(0xff1959f4),60,54,false));
    g.strokePath(mark,juce::PathStrokeType(6.5f,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    text(g,"Matale Tune",{82,13,290,34},28,ink,true);
    text(g,"PRO VOCAL PROCESSOR / v0.13.0",{83,46,300,18},10.5f,muted);
    text(g,"AUTO CHARACTER",{getWidth()-650,7,210,17},10,muted,true);

    g.setColour(line);g.drawHorizontalLine(getHeight()-35,20.f,float(getWidth()-20));
    if(manualOpen){paintManual(g);return;}
    for(auto r:{analysisBounds,outputBounds,autoBounds,smoothBounds})card(g,r);
    auto a=analysisBounds;float hz=processor.displayedPitch.load(),confidence=processor.pitchConfidence.load();int midi=hz>0?juce::roundToInt(69+12*std::log2(hz/440.f)):-1;
    text(g,"Vocal Analysis",{a.getX()+20,a.getY()+16,220,26},19,ink,true);text(g,hz>0?"Live / voiced":"No voiced input",{a.getRight()-130,a.getY()+18,112,22},11,hz>0?green:muted,false,juce::Justification::centredRight);
    auto ring=juce::Rectangle<float>(100,100).withCentre({float(a.getX()+73),float(a.getCentreY()+10)});g.setColour(juce::Colour(0xffcfedf9));g.drawEllipse(ring,8);juce::Path arc;arc.addCentredArc(ring.getCentreX(),ring.getCentreY(),50,50,0,-2.5f,-2.5f+5*juce::jlimit(0.f,1.f,confidence),true);g.setColour(blue);g.strokePath(arc,juce::PathStrokeType(8));text(g,noteName(midi),{a.getX()+23,a.getCentreY()-10,100,36},28,ink,true,juce::Justification::centred);text(g,hz>0?juce::String(hz,1)+" Hz":"No pitch",{a.getX()+23,a.getCentreY()+29,100,20},11,muted,false,juce::Justification::centred);
    int sx=a.getX()+146,sw=juce::jlimit(140,180,a.getWidth()/4),sy=a.getY()+70;int min=processor.minNote.load(),max=processor.maxNote.load(),median=processor.medianNote.load();juce::String reg=median<0?"More notes":median<48?"Bass":median<55?"Baritone":median<60?"Tenor":median<65?"Alto":median<69?"Mezzo":"Soprano";juce::String titles[]={"Detected note","Confidence","Vocal range","Register (est.)","Energy / RMS"};juce::String vals[]={noteName(midi),hz>0?juce::String(int(confidence*100))+"%":"--",min<=max?noteName(min)+" - "+noteName(max):"--",reg,juce::String(processor.inputRms.load()*100,1)+"% FS"};for(int i=0;i<5;++i){text(g,titles[i],{sx,sy+i*25,sw-67,20},11,muted);text(g,vals[i],{sx+sw-85,sy+i*25,85,20},11,ink,false,juce::Justification::centredRight);}
    auto plot=juce::Rectangle<int>(sx+sw+16,a.getY()+74,a.getRight()-sx-sw-36,a.getHeight()-128);text(g,"LIVE OUTPUT SPECTRUM",{plot.getX(),plot.getY()-22,plot.getWidth(),18},9,muted);g.setColour(line);g.drawHorizontalLine(plot.getBottom(),float(plot.getX()),float(plot.getRight()));juce::Path path;for(int i=0;i<int(spectrum.size());++i){float x=plot.getX()+plot.getWidth()*float(i)/float(spectrum.size()-1),y=plot.getBottom()-spectrum[size_t(i)]*plot.getHeight();if(i==0)path.startNewSubPath(x,y);else path.lineTo(x,y);}for(int n=1;n<4;++n){g.setColour(line.withAlpha(.65f));g.drawHorizontalLine(plot.getY()+plot.getHeight()*n/4,float(plot.getX()),float(plot.getRight()));}
    auto area=path;area.lineTo(float(plot.getRight()),float(plot.getBottom()));area.lineTo(float(plot.getX()),float(plot.getBottom()));area.closeSubPath();
    g.setGradientFill(juce::ColourGradient(blue.withAlpha(.22f),float(plot.getX()),float(plot.getY()),cyan.withAlpha(.015f),float(plot.getX()),float(plot.getBottom()),false));g.fillPath(area);
    g.setColour(blue);g.strokePath(path,juce::PathStrokeType(1.6f));meter(g,{float(plot.getX()),float(a.getBottom()-32),float(juce::jmax(20,plot.getWidth()-80)),7},processor.inputLevel.load());text(g,decibel(processor.inputLevel.load())+" dBFS",{plot.getRight()-80,a.getBottom()-41,80,24},10,ink,false,juce::Justification::centredRight);
    auto o=outputBounds;text(g,"Output",{o.getX()+20,o.getY()+16,160,26},19,ink,true);text(g,decibel(processor.outputLevel.load())+" dBFS",{o.getX()+20,o.getY()+62,o.getWidth()-40,44},29,ink,true);meter(g,{float(o.getX()+20),float(o.getY()+121),float(o.getWidth()-40),11},processor.outputLevel.load());float lufs=processor.momentaryLufs.load();text(g,"Momentary LUFS",{o.getX()+20,o.getBottom()-73,130,20},10,muted);text(g,lufs>-179?juce::String(lufs,1):"--",{o.getX()+20,o.getBottom()-49,100,24},16,ink,true);text(g,"Sample peak",{o.getX()+150,o.getBottom()-73,110,20},10,muted);text(g,decibel(processor.samplePeak.load())+" dBFS",{o.getX()+150,o.getBottom()-27,110,20},12,ink);
    for(int c=0;c<2;++c){float level=c?processor.rightLevel.load():processor.leftLevel.load();int x=o.getRight()-42+c*18,y=o.getY()+151,h=juce::jmax(25,o.getHeight()-175);g.setColour(line);g.fillRoundedRectangle(float(x),float(y),8,float(h),4);float fill=juce::jlimit(0.f,1.f,(matale::db(level)+60)/60);g.setColour(c?cyan:green);g.fillRoundedRectangle(float(x),float(y+h)-h*fill,8,h*fill,4);text(g,c?"R":"L",{x-2,y-18,14,16},9,muted);}
    auto b=autoBounds;text(g,"Auto Mode",{b.getX()+20,b.getY()+16,220,28},20,ink,true);text(g,"Choose a character above, then press AUTO.",{b.getX()+20,b.getY()+44,b.getWidth()-40,20},11,muted);int state=processor.autoState.load();if(state==1){meter(g,{float(b.getX()+20),float(b.getBottom()-30),float(b.getWidth()-40),8},matale::gain(processor.progress.load()*60-60));}else if(b.getHeight()>300)text(g,"Dry analysis | adaptive EQ / compression | natural pitch",{b.getX()+20,b.getBottom()-32,b.getWidth()-40,22},11,muted);
    auto s=smoothBounds;text(g,"Vocal Smooth",{s.getX()+20,s.getY()+16,s.getWidth()-164,28},20,ink,true);text(g,"Dynamic harshness reduction and tonal shaping.",{s.getX()+20,s.getY()+49,s.getWidth()-40,18},11,muted);auto result=juce::Rectangle<int>(s.getX()+18,s.getBottom()-86,s.getWidth()-36,66);g.setColour(juce::Colour(0xffeaf7ff));g.fillRoundedRectangle(result.toFloat(),10);text(g,"Measured processing",result.reduced(12).withHeight(22),12,blue,true);text(g,"Smooth "+juce::String(processor.smoothReduction.load(),1)+" dB  /  De-Esser "+juce::String(processor.deessReduction.load(),1)+" dB",{result.getX()+12,result.getY()+38,result.getWidth()-24,24},11,muted);
}
