#include "PluginEditor.h"
namespace {
const juce::Colour ink(0xff102d53), muted(0xff7386a4), blue(0xff1685ff), cyan(0xff52d8f5);
const juce::Colour line(0xffe3edf6), green(0xff13cda0);
constexpr int quickIds[] = {7, 4, 8, 6, 31, 28};
const char* quickNames[] = {"Pitch correction", "Retune speed", "Dynamics threshold", "Formant colour", "Reverb", "Delay"};
const char* quickUnits[] = {"%", " ms", " dB", "", "", ""};
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

MataleTuneAudioProcessorEditor::MataleTuneAudioProcessorEditor(MataleTuneAudioProcessor& p)
    : AudioProcessorEditor(&p), processor(p) {
    setLookAndFeel(&look);
    addAndMakeVisible(autoButton); addAndMakeVisible(manualButton);
    addAndMakeVisible(power); addAndMakeVisible(status);
    autoButton.setComponentID("auto-orb"); manualButton.setComponentID("manual");
    autoButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    autoButton.setTooltip("Listen to three seconds of vocal input and apply the existing Auto analysis.");
    autoButton.onClick = [this] { processor.requestAuto.store(true); autoButton.setEnabled(false); };
    manualButton.onClick = [this] { selectPage(1); };
    powerAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(p.parameters, "auto", power);
    const char* names[] = {"Home", "Pitch", "Dynamics", "EQ", "Effects"};
    for (int i=0; i<5; ++i) {
        addAndMakeVisible(navigation[i]); navigation[i].setButtonText(names[i]);
        navigation[i].onClick = [this,i] { selectPage(i); };
    }
    for (auto& page : pages) addChildComponent(page);
    for (int i=0; i<matale::parameterCount; ++i) {
        if (i==3) continue;
        const auto& d = matale::parameterList[i];
        pages[d.page].addAndMakeVisible(labels[i]); labels[i].setText(d.name, juce::dontSendNotification);
        labels[i].setFont(juce::FontOptions(13.f));
        if (i<3) {
            juce::StringArray opts;
            if (i==0) opts={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
            if (i==1) opts={"Chromatic","Major","Minor","Major Pentatonic","Minor Pentatonic"};
            if (i==2) opts={"Natural","Hard Tune"};
            choices[i].addItemList(opts,1); pages[0].addAndMakeVisible(choices[i]);
            comboAttachments.push_back(std::make_unique<CA>(p.parameters,d.id,choices[i]));
        } else {
            pages[d.page].addAndMakeVisible(sliders[i]);
            sliders[i].setSliderStyle(juce::Slider::LinearHorizontal);
            sliders[i].setTextBoxStyle(juce::Slider::TextBoxRight,false,76,24);
            sliderAttachments.push_back(std::make_unique<SA>(p.parameters,d.id,sliders[i]));
        }
    }
    for (int i=0; i<6; ++i) {
        addAndMakeVisible(quickSliders[i]);
        quickSliders[i].setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        quickSliders[i].setTextBoxStyle(juce::Slider::TextBoxBelow,false,100,24);
        quickSliders[i].setTextValueSuffix(quickUnits[i]);
        quickSliders[i].setTitle(quickNames[i]);
        sliderAttachments.push_back(std::make_unique<SA>(p.parameters,matale::parameterList[quickIds[i]].id,quickSliders[i]));
    }
    status.setFont(juce::FontOptions(13.f)); status.setColour(juce::Label::textColourId, muted);
    status.setText("Ready. Choose key / scale in Pitch, then sing and press AUTO.",juce::dontSendNotification);
    setResizable(true,true); setResizeLimits(1180,820,1800,1200); setSize(1440,940);
    selectPage(0); startTimerHz(24);
}
MataleTuneAudioProcessorEditor::~MataleTuneAudioProcessorEditor() { stopTimer(); setLookAndFeel(nullptr); }
void MataleTuneAudioProcessorEditor::selectPage(int page) {
    selectedPage=page;
    for (int i=0;i<5;++i) navigation[i].setToggleState(i==page,juce::dontSendNotification);
    for (int i=0;i<4;++i) pages[i].setVisible(page==i+1);
    for (auto& slider:quickSliders) slider.setVisible(page==0);
    autoButton.setVisible(page==0); manualButton.setVisible(page==0);
    resized(); repaint();
}
void MataleTuneAudioProcessorEditor::resized() {
    const int x=148, gap=20, width=getWidth()-x-24;
    const int upper=juce::jmax(200,getHeight()/4-14), outputWidth=width/3;
    analysisBounds={x,106,width-outputWidth-gap,upper};
    outputBounds={analysisBounds.getRight()+gap,106,outputWidth,upper};
    const int middleY=analysisBounds.getBottom()+gap, bottom=getHeight()-54;
    const int quickHeight=juce::jlimit(156,200,getHeight()/5);
    rackBounds={x,bottom-quickHeight,width,quickHeight};
    autoBounds={x,middleY,(width-gap)*2/3,rackBounds.getY()-gap-middleY};
    summaryBounds={autoBounds.getRight()+gap,middleY,width-autoBounds.getWidth()-gap,autoBounds.getHeight()};
    power.setBounds(getWidth()-190,35,165,28);
    for(int i=0;i<5;++i) navigation[i].setBounds(14,132+i*62,112,48);
    status.setBounds(x,getHeight()-38,width,25);
    int orb=juce::jmin(220,autoBounds.getHeight()-52);
    autoButton.setBounds(autoBounds.getRight()-orb-30,autoBounds.getCentreY()-orb/2,orb,orb);
    manualButton.setBounds(summaryBounds.getX()+24,summaryBounds.getBottom()-66,summaryBounds.getWidth()-48,38);
    for(int i=0;i<6;++i) {
        int cellWidth=width/6;
        quickSliders[i].setBounds(x+i*cellWidth+20,rackBounds.getY()+38,cellWidth-40,quickHeight-48);
    }
    for(int page=0;page<4;++page) {
        auto bounds=juce::Rectangle<int>(x,middleY,width,bottom-middleY);
        pages[page].setBounds(bounds.reduced(20,46));
        int n=0, cellWidth=pages[page].getWidth()/3;
        int count=0;for(int i=0;i<matale::parameterCount;++i)if(i!=3&&matale::parameterList[i].page==page)++count;
        const int rows=(count+2)/3, rowHeight=juce::jmin(100,pages[page].getHeight()/rows);
        for(int i=0;i<matale::parameterCount;++i)if(i!=3&&matale::parameterList[i].page==page) {
            int cx=(n%3)*cellWidth+12,cy=(n/3)*rowHeight;
            labels[i].setBounds(cx,cy,cellWidth-32,23);
            if(i<3) choices[i].setBounds(cx,cy+26,cellWidth-32,30);
            else sliders[i].setBounds(cx,cy+23,cellWidth-32,32);
            ++n;
        }
    }
}
void MataleTuneAudioProcessorEditor::timerCallback() {
    int s=processor.autoState.load();
    if(s==2) {processor.applyAutoResult();status.setText("Auto applied. Key / scale preserved. Fine-tune by ear.",juce::dontSendNotification);}
    else if(s==1) status.setText("Listening: sing for 3 seconds  |  "+juce::String(int(processor.progress.load()*100))+"%",juce::dontSendNotification);
    else if(s==3) status.setText("Not enough voiced audio. Sing into the input and retry.",juce::dontSendNotification);
    autoButton.setEnabled(s!=1); autoButton.setButtonText(s==1?"LISTENING":"AUTO"); repaint();
}
void MataleTuneAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xfff3f8fd));
    g.setColour(juce::Colour(0xffe8f0f8));g.drawHorizontalLine(86,0,float(getWidth()));g.drawVerticalLine(138,86,float(getHeight()));
    juce::Path logo;logo.startNewSubPath(29,61);logo.lineTo(39,30);logo.lineTo(54,52);logo.lineTo(68,30);logo.lineTo(78,61);
    g.setGradientFill(juce::ColourGradient(cyan,29,30,blue,78,61,false));g.strokePath(logo,juce::PathStrokeType(9,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    text(g,"Matale Tune",{103,20,380,34},29,ink,true);
    text(g,"Pro Vocal Processor",{105,54,340,22},14,muted);
    text(g,"Better Voice Better Music",{getWidth()-480,37,270,24},14,muted,false,juce::Justification::centredRight);
    text(g,"VOCAL TOOLS",{24,100,110,20},10,muted,true);
    text(g,"2048 frames",{22,getHeight()-87,110,20},12,muted);
    const double rate=processor.getSampleRate();
    text(g,rate>0?juce::String(2048000.0/rate,1)+" ms DSP":"Awaiting audio",{22,getHeight()-64,110,20},12,muted);
    card(g,analysisBounds);card(g,outputBounds);
    auto a=analysisBounds;
    text(g,"Vocal Analysis",{a.getX()+24,a.getY()+16,210,24},17,ink,true);
    bool voiced=processor.displayedPitch.load()>0;
    g.setColour(voiced?green:muted);g.fillEllipse(float(a.getRight()-73),float(a.getY()+25),7,7);
    text(g,voiced?"Voiced":"Idle",{a.getRight()-59,a.getY()+15,48,24},12,muted);
    auto ring=juce::Rectangle<float>(112,112).withCentre({float(a.getX()+83),float(a.getCentreY()+12)});
    g.setColour(juce::Colour(0xffe6f4fc));g.drawEllipse(ring,9);
    juce::Path arc;arc.addCentredArc(ring.getCentreX(),ring.getCentreY(),56,56,0,-2.5f,voiced?2.5f:-1.1f,true);
    g.setColour(blue);g.strokePath(arc,juce::PathStrokeType(9,juce::PathStrokeType::curved,juce::PathStrokeType::rounded));
    float hz=processor.displayedPitch.load();juce::String note="--";
    if(hz>0&&std::isfinite(hz)) {
        int midi=juce::roundToInt(69+12*std::log2(hz/440.f));
        const char* names[]={"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
        note=juce::String(names[(midi%12+12)%12])+juce::String(midi/12-1);
    }
    text(g,note,{a.getX()+28,a.getCentreY()-15,110,42},32,ink,true,juce::Justification::centred);
    text(g,hz>0?juce::String(hz,1)+" Hz":"No pitch",{a.getX()+28,a.getCentreY()+30,110,22},12,muted,false,juce::Justification::centred);
    int waveX=a.getX()+173,waveW=a.getRight()-waveX-25;
    text(g,"LIVE INPUT",{waveX,a.getY()+57,waveW,18},11,muted,true);
    auto wave=juce::Rectangle<int>(waveX,a.getY()+83,waveW,a.getHeight()-137);
    g.setColour(line);g.drawHorizontalLine(wave.getCentreY(),float(wave.getX()),float(wave.getRight()));
    juce::Path path;
    for(int i=0;i<256;++i) {
        float wx=wave.getX()+wave.getWidth()*i/255.f;
        float wy=wave.getCentreY()-juce::jlimit(-1.f,1.f,processor.scope[size_t(i)].load())*wave.getHeight()*.45f;
        if(i==0)path.startNewSubPath(wx,wy);else path.lineTo(wx,wy);
    }
    g.setColour(blue);g.strokePath(path,juce::PathStrokeType(1.5f));
    text(g,"Input",{waveX,a.getBottom()-39,55,20},12,muted);
    meter(g,{float(waveX+58),float(a.getBottom()-30),float(juce::jmax(20,waveW-155)),8},processor.inputLevel.load());
    text(g,juce::String(matale::db(processor.inputLevel.load()),1)+" dBFS",{a.getRight()-112,a.getBottom()-40,90,22},12,ink,false,juce::Justification::centredRight);
    auto o=outputBounds;
    text(g,"Output",{o.getX()+25,o.getY()+18,200,24},17,ink,true);
    text(g,juce::String(matale::db(processor.outputLevel.load()),1)+" dBFS",{o.getX()+25,o.getY()+55,o.getWidth()-50,44},30,ink,true);
    meter(g,{float(o.getX()+25),float(o.getY()+113),float(o.getWidth()-50),12},processor.outputLevel.load());
    text(g,"Peak level  |  Linked mono / stereo",{o.getX()+25,o.getBottom()-64,o.getWidth()-50,22},12,muted);
    text(g,power.getToggleState()?"Processing enabled":"Latency-aligned bypass",{o.getX()+25,o.getBottom()-40,o.getWidth()-50,22},13,power.getToggleState()?green:muted);
    if(selectedPage==0) {
        card(g,autoBounds);card(g,summaryBounds);
        auto b=autoBounds;
        text(g,"Auto Mode",{b.getX()+24,b.getY()+20,b.getWidth()-48,28},19,ink,true);
        text(g,"Listen. Analyse. Fine-tune.",{b.getX()+24,b.getY()+55,240,22},13,muted);
        const char* items[]={"Vocal input analysis","Dynamics / makeup","Retune / humanize","EQ recommendations","Reverb & delay"};
        for(int i=0;i<5;++i) {
            int iy=b.getY()+99+i*28;
            g.setColour(blue);g.fillEllipse(float(b.getX()+25),float(iy+5),6,6);
            text(g,items[i],{b.getX()+42,iy,230,22},13,ink);
        }
        auto s=summaryBounds;
        text(g,"Your voice. Your control.",{s.getX()+24,s.getY()+23,s.getWidth()-48,28},17,ink,true);
        text(g,"Start with your key and scale.",{s.getX()+24,s.getY()+72,s.getWidth()-48,24},13,muted);
        text(g,"Auto listens for three seconds.",{s.getX()+24,s.getY()+104,s.getWidth()-48,24},13,muted);
        text(g,"Adjust every parameter by ear.",{s.getX()+24,s.getY()+136,s.getWidth()-48,24},13,muted);
        text(g,"Key / scale stay unchanged.",{s.getX()+24,s.getY()+160,s.getWidth()-48,20},12,blue);
        for(int i=0;i<6;++i) {
            int cw=rackBounds.getWidth()/6;
            auto c=juce::Rectangle<int>(rackBounds.getX()+i*cw,rackBounds.getY(),cw-8,rackBounds.getHeight());
            card(g,c);text(g,quickNames[i],c.reduced(14).withHeight(24),13,ink,true,juce::Justification::centred);
        }
    } else {
        auto panel=juce::Rectangle<int>(analysisBounds.getX(),analysisBounds.getBottom()+20,
                                       getWidth()-analysisBounds.getX()-24,getHeight()-54-analysisBounds.getBottom()-20);
        card(g,panel);
        const char* headings[]={"Pitch correction", "Dynamics", "5-Band Equalizer", "Delay / Reverb"};
        text(g,headings[selectedPage-1],panel.reduced(28,12).withHeight(27),19,ink,true);
    }
}
