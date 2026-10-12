// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 hsduren and Traka contributors
#include "Studio.h"
namespace {
Colour bg(0xff14171c), panel(0xff1d2229), line(0xff303740), text(0xffe3e7ef), muted(0xff85919f), accent(0xffa8d5b5);
Colour trackColour(int i) { const uint32 cols[] = {0xffa8d5b5,0xffa7bbdc,0xffd8b38b,0xffc6add9,0xff83c8ca}; return Colour(cols[i % 5]); }
void label(Graphics& g, const String& s, Rectangle<int> r, float size = 13, Colour c = text) {
    g.setColour(c); g.setFont(FontOptions(size)); g.drawText(s,r,Justification::centredLeft);
}
class LicenseView : public Component {
public:
    TextEditor document;
    HyperlinkButton source;
    LicenseView(const String& info, const String& sourceLabel) : source(sourceLabel, URL(TRAKA_SOURCE_URL)) {
        document.setMultiLine(true);document.setReadOnly(true);document.setScrollbarsShown(true);document.setText(info.replace("\f","\n"),false);
        addAndMakeVisible(document);addAndMakeVisible(source);setSize(740,560);
    }
    void resized() override {source.setBounds(12,8,getWidth()-24,30);document.setBounds(12,46,getWidth()-24,getHeight()-58);}
};
String timeString(double s) { return String::formatted("%02d:%02d.%02d", (int)s/60, (int)s%60, (int)(s*100)%100); }
}
Timeline::Timeline(Studio& s) : studio(s) { formats.registerBasicFormats(); }
double Timeline::secondsAt(float x) const { return jmax(0.0, (x-header) * studio.viewSeconds / jmax(1,getWidth()-header)); }
float Timeline::xAt(double s) const { return header + float(s / studio.viewSeconds * (getWidth()-header)); }
void Timeline::paint(Graphics& g) {
    g.fillAll(bg);
    auto& session = studio.session;
    g.setColour(panel); g.fillRect(0,0,header,getHeight()); g.fillRect(0,0,getWidth(),36);
    label(g,studio.l("AUDIO TRACKS", "音频轨道"),{16,0,header-16,36},11,muted);
    double step = studio.viewSeconds > 90 ? 10 : studio.viewSeconds > 40 ? 5 : studio.viewSeconds > 20 ? 2 : 1;
    for (double s=0; s<=studio.viewSeconds; s+=step) {
        int x=roundToInt(xAt(s)); g.setColour(line.withAlpha(0.7f)); g.drawVerticalLine(x,36,(float)getHeight());
        label(g,String(s,0)+"s",{x+5,0,50,36},11,muted);
    }
    int row=0;
    for (auto t : te::getAudioTracks(*session.edit)) {
        int y=36+row*100; auto col=trackColour(row);
        if (row==studio.selectedTrack) { g.setColour(Colour(0xff283039)); g.fillRect(0,y,header,100); }
        g.setColour(col); g.fillRect(0,y,3,100);
        label(g,String(row+1).paddedLeft('0',2),{14,y+10,28,24},12,muted);
        label(g,t->getName(),{48,y+10,151,24},14);
        for (int b=0; b<2; ++b) {
            bool on=b==0?t->isMuted(false):t->isSolo(false);
            Rectangle<int> r(48+b*34,y+44,27,24);
            g.setColour(on?col:line); g.fillRoundedRectangle(r.toFloat(),4);
            g.setColour(on?bg:text); g.setFont(FontOptions(12)); g.drawText(b==0?"M":"S",r,Justification::centred);
        }
        auto v=t->getVolumePlugin();
        label(g,v?String(v->getVolumeDb(),1)+" dB":"",{120,y+44,80,24},11,muted);
        if (session.edit->getTransport().isRecording() && row==studio.selectedTrack) {
            label(g,studio.l("RECORDING", "正在录音"),{48,y+73,150,20},10,Colours::salmon);
            g.setColour(Colours::salmon.withAlpha(0.13f)); g.fillRect(header,y+4,getWidth()-header,92);
        }
        for (auto c : t->getClips()) {
            auto pos=c->getPosition(); float x=xAt(pos.time.getStart().inSeconds()); float right=xAt(pos.time.getEnd().inSeconds());
            Rectangle<float> r(x,y+8.f,jmax(2.f,right-x),84.f);
            g.saveState(); g.reduceClipRegion(Rectangle<int>(header,y,getWidth()-header,100));
            bool isSelected=studio.selected.get()==c;
            g.setColour(col.withAlpha(t->isMuted(false)?0.18f:0.27f)); g.fillRoundedRectangle(r,5);
            g.setColour(col.withAlpha(isSelected?1.f:0.5f)); g.drawRoundedRectangle(r,5,isSelected?2.f:1.f);
            auto source=c->getSourceFileReference().getFile();
            auto key=source.getFullPathName();
            if (source.existsAsFile()) {
                if (!thumbnails.count(key)) {
                    auto thumb=std::make_unique<AudioThumbnail>(512,formats,cache);
                    thumb->setSource(new FileInputSource(source)); thumbnails[key]=std::move(thumb);
                }
                auto& thumb=*thumbnails[key];
                double start=pos.offset.inSeconds();
                g.setColour(col.withAlpha(0.8f));
                thumb.drawChannels(g,r.reduced(5,7).withTrimmedTop(18).toNearestInt(),start,start+pos.time.getLength().inSeconds(),0.85f);
            } else label(g,studio.l("Missing audio", "音频文件丢失"),r.toNearestInt().reduced(8),12,Colours::salmon);
            label(g,c->getName(),r.toNearestInt().reduced(9).withHeight(18),12,col.brighter(0.4f));
            if(isSelected) { g.setColour(col); g.fillRect(x+3,y+36.f,2.f,22.f); g.fillRect(right-5,y+36.f,2.f,22.f); }
            g.restoreState();
        }
        g.setColour(line); g.drawHorizontalLine(y+99,0,(float)getWidth()); ++row;
    }
    if (session.length()==0) {
        label(g,studio.l("Drop audio here to start", "拖入音频，开始创作"),{header+30,65,getWidth()-header-60,28},18,muted);
        label(g,studio.l("WAV, AIFF, MP3, FLAC · or open the demo", "WAV、AIFF、MP3、FLAC · 也可打开示例"),{header+30,98,getWidth()-header-60,24},12,muted);
    }
    float playX=xAt(session.edit->getTransport().getPosition().inSeconds());
    if(playX>=header && playX<getWidth()) { g.setColour(text); g.fillRect(playX,22.f,1.3f,(float)getHeight()-22); g.fillEllipse(playX-3,23,6,6); }
}
void Timeline::mouseDown(const MouseEvent& e) {
    if (studio.busy || studio.session.edit->getTransport().isRecording()) return;
    studio.grabKeyboardFocus(); dragged=nullptr;
    if(e.y<36 && e.x>=header) { studio.session.edit->getTransport().setPosition(tp(secondsAt((float)e.x))); return; }
    int row=rowAt(e.y); if(!studio.session.track(row))return;
    studio.selectTrack(row);
    if(e.x<header) {
        int ry=e.y-36-row*100;
        auto t=studio.session.track(row);
        if(ry>=44 && ry<=68) {
            studio.session.begin("Mute / solo");
            if(e.x>=48 && e.x<75)t->setMute(!t->isMuted(false));
            if(e.x>=82 && e.x<109)t->setSolo(!t->isSolo(false));
        }
        repaint();return;
    }
    studio.selected=nullptr;
    for(auto c:studio.session.track(row)->getClips()) {
        auto pos=c->getPosition(); float left=xAt(pos.time.getStart().inSeconds()), right=xAt(pos.time.getEnd().inSeconds());
        if(e.x>=left && e.x<=right) {
            studio.selected=c; dragged=c; original=pos;
            dragMode=e.x-left<9?1:right-e.x<9?2:0;
            studio.session.begin(dragMode==0?"Move clip":"Trim clip");
            break;
        }
    }
    if(!studio.selected)studio.session.edit->getTransport().setPosition(tp(secondsAt((float)e.x)));
    studio.refresh();
}
void Timeline::mouseDrag(const MouseEvent& e) {
    if(!dragged)return;
    double delta=e.getDistanceFromDragStartX()*studio.viewSeconds/jmax(1,getWidth()-header);
    if(dragMode==0) dragged->setStart(tp(jmax(0.0,original.time.getStart().inSeconds()+delta)),false,true);
    else if(dragMode==1) {
        double start=jlimit(jmax(0.0,original.time.getStart().inSeconds()-original.offset.inSeconds()),original.time.getEnd().inSeconds()-0.02,original.time.getStart().inSeconds()+delta);
        dragged->setPosition({{tp(start),original.time.getEnd()},td(original.offset.inSeconds()+start-original.time.getStart().inSeconds())});
    } else {
        double maxEnd=original.time.getEnd().inSeconds();
        if(auto w=dynamic_cast<te::WaveAudioClip*>(dragged.get())) {
            te::AudioFile af(studio.session.engine,w->getSourceFileReference().getFile());
            maxEnd=original.time.getStart().inSeconds()+af.getLength()-original.offset.inSeconds();
        }
        dragged->setEnd(tp(jlimit(original.time.getStart().inSeconds()+0.02,maxEnd,original.time.getEnd().inSeconds()+delta)),true);
    }
    repaint();
}
void Timeline::mouseUp(const MouseEvent& e) {
    if(dragged && dragMode==0 && e.mouseWasDraggedSinceMouseDown()) {
        int target=rowAt(e.y);
        if(auto t=studio.session.track(target); t && target!=studio.selectedTrack) { dragged->moveTo(*t); studio.selectTrack(target); }
    }
    dragged=nullptr; studio.refresh();
}
void Timeline::filesDropped(const StringArray& files,int x,int y) {
    if(studio.busy || studio.session.edit->getTransport().isRecording())return;
    int row=jlimit(0,studio.session.numTracks()-1,rowAt(y));
    for(auto& f:files) { studio.importFile(File(f),row,secondsAt((float)x)); row=studio.session.numTracks(); }
}
Studio::Studio() {
    chinese=SystemStats::getUserLanguage().startsWithIgnoreCase("zh");
    auto prefs=File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Traka/language.txt").loadFileAsString().trim();
    if(prefs=="en")chinese=false; if(prefs=="zh")chinese=true;
    theme.setColour(ResizableWindow::backgroundColourId,bg);
    theme.setColour(TextButton::buttonColourId,panel); theme.setColour(TextButton::buttonOnColourId,accent);
    theme.setColour(TextButton::textColourOffId,text); theme.setColour(TextButton::textColourOnId,bg);
    theme.setColour(ComboBox::backgroundColourId,panel); theme.setColour(ComboBox::outlineColourId,line);
    theme.setColour(ComboBox::textColourId,text); theme.setColour(PopupMenu::backgroundColourId,panel);
    theme.setColour(PopupMenu::textColourId,text); theme.setColour(Slider::thumbColourId,accent);
    theme.setColour(Slider::trackColourId,accent); theme.setColour(Slider::backgroundColourId,line);
    theme.setColour(Label::textColourId,text);
    theme.setColour(TextEditor::backgroundColourId,panel); theme.setColour(TextEditor::textColourId,text);
    theme.setColour(TextEditor::outlineColourId,line); setLookAndFeel(&theme);
    for(auto c : std::initializer_list<Component*>{&newButton,&openButton,&saveButton,&importButton,&addButton,&demoButton,&settingsButton,
        &openSourceButton,&exportButton,&playButton,&stopButton,&recordButton,&splitButton,&deleteButton,&loopButton,&title,&clock,&message,&trackLabel,
        &nameEditor,&volumeLabel,&panLabel,&volume,&pan,&zoom,&input,&language,&viewport})addAndMakeVisible(c);
    viewport.setViewedComponent(&timeline,false); viewport.setScrollBarsShown(true,false);
    title.setFont(FontOptions(23)); clock.setFont(FontOptions("Menlo",22,Font::plain)); message.setFont(FontOptions(12));
    message.setColour(Label::textColourId,muted); title.setText("Traka",dontSendNotification);
    nameEditor.onReturnKey=[this]{ if(auto t=session.track(selectedTrack)) {session.begin("Rename track");t->setName(nameEditor.getText());timeline.repaint();} grabKeyboardFocus();};
    nameEditor.onFocusLost=nameEditor.onReturnKey;
    volume.setRange(-60,6,0.1); volume.setSliderStyle(Slider::LinearHorizontal); volume.setTextBoxStyle(Slider::TextBoxRight,false,62,24); volume.setTextValueSuffix(" dB");
    pan.setRange(-1,1,0.01); pan.setSliderStyle(Slider::LinearHorizontal); pan.setTextBoxStyle(Slider::TextBoxRight,false,62,24);
    volume.onDragStart=[this]{session.begin("Track volume");}; pan.onDragStart=[this]{session.begin("Track pan");};
    volume.onValueChange=[this]{if(auto t=session.track(selectedTrack))t->getVolumePlugin()->setVolumeDb((float)volume.getValue());};
    pan.onValueChange=[this]{if(auto t=session.track(selectedTrack))t->getVolumePlugin()->setPan((float)pan.getValue());};
    zoom.setRange(5,300,1);zoom.setValue(viewSeconds);zoom.setSliderStyle(Slider::LinearHorizontal);zoom.setTextBoxStyle(Slider::TextBoxRight,false,55,24);zoom.setTextValueSuffix(" s");
    zoom.onValueChange=[this]{viewSeconds=zoom.getValue();timeline.repaint();};
    language.addItem("English",1);language.addItem(String::fromUTF8("简体中文"),2);language.addItem("System",3);
    language.setSelectedId(prefs=="en"?1:prefs=="zh"?2:3,dontSendNotification);
    language.onChange=[this]{auto id=language.getSelectedId();chinese=id==2 || (id==3 && SystemStats::getUserLanguage().startsWithIgnoreCase("zh"));
        auto f=File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Traka/language.txt");f.getParentDirectory().createDirectory();f.replaceWithText(id==1?"en":id==2?"zh":"system");updateLanguage();};
    newButton.onClick=[this]{session.stop();te::EditFileOperations(*session.edit).save(false,true,false);selected=nullptr;session.newProject();resetView();status(l("New session · saved locally", "新项目 · 已保存在本机"));};
    openButton.onClick=[this]{chooseOpen();};saveButton.onClick=[this]{chooseSave();};importButton.onClick=[this]{chooseImport();};
    addButton.onClick=[this]{session.addTrack();selectTrack(session.numTracks()-1);refresh();};
    demoButton.onClick=[this]{session.stop();te::EditFileOperations(*session.edit).save(false,true,false);selected=nullptr;auto r=session.makeDemo();resetView();status(r.wasOk()?l("Demo ready · press Play", "示例已就绪 · 点击播放"):r.getErrorMessage());};
    openSourceButton.onClick=[this]{
        auto notices=File::getSpecialLocation(File::currentApplicationFile).getChildFile("Contents/Resources/OpenSource");
        String info="Traka " TRAKA_VERSION "\nCopyright (C) 2026 hsduren and Traka contributors\n\n";
        info+=l("Source and version tags: ", "源码和版本标签：")+String(TRAKA_SOURCE_URL)+"\n\n";
        info+=l("Traka code: AGPL-3.0-or-later. Tracktion Engine: GPL-3.0-or-later. JUCE: AGPLv3.\nYou may copy, modify and redistribute under these licenses. This program comes with no warranty.\n\n", "Traka 代码：AGPL-3.0-or-later；Tracktion Engine：GPL-3.0-or-later；JUCE：AGPLv3。\n可按许可证复制、修改和再分发；本软件不提供担保。\n\n");
        for(auto file:notices.findChildFiles(File::findFiles,false))info+="\n===== "+file.getFileName()+" =====\n"+file.loadFileAsString()+"\n";
        auto content=new LicenseView(info,l("Open Traka source repository", "打开 Traka 源码仓库"));
        DialogWindow::LaunchOptions options;options.dialogTitle=l("Traka — Open source & licenses", "Traka — 开源与许可");
        options.dialogBackgroundColour=panel;options.content.setOwned(content);options.launchAsync();
    };
    settingsButton.onClick=[this]{audioSettings();};exportButton.onClick=[this]{chooseExport();};
    playButton.onClick=[this]{togglePlay();};stopButton.onClick=[this]{session.stop();refresh();};recordButton.onClick=[this]{toggleRecord();};
    splitButton.onClick=[this]{splitClip();};deleteButton.onClick=[this]{deleteClip();};
    loopButton.onClick=[this]{session.edit->getTransport().setLoopRange({tp(0),tp(jmax(1.0,session.length()))});session.edit->getTransport().looping=loopButton.getToggleState();};
    setWantsKeyboardFocus(true);updateLanguage();selectTrack(0);refresh();
    status(l("Ready · import audio or try the demo", "就绪 · 导入音频或打开示例"));startTimerHz(20);setSize(1180,760);
}
Studio::~Studio() { stopTimer();render.reset();setLookAndFeel(nullptr); }
String Studio::l(const char* en,const char* zh) const {return String::fromUTF8(chinese?zh:en);}
void Studio::status(const String& s){messageText=s;message.setText(s,dontSendNotification);}
void Studio::updateLanguage() {
    newButton.setButtonText(l("New", "新建"));openButton.setButtonText(l("Open", "打开"));saveButton.setButtonText(l("Save", "保存"));
    importButton.setButtonText(l("Import audio", "导入音频"));addButton.setButtonText(l("+ Audio track", "+ 音频轨"));demoButton.setButtonText(l("Demo", "示例"));
    settingsButton.setButtonText(l("Audio settings", "音频设置"));exportButton.setButtonText(l("Export WAV", "导出 WAV"));
    openSourceButton.setButtonText(l("Open source", "开源与许可"));
    stopButton.setButtonText(l("Stop", "停止"));splitButton.setButtonText(l("Split", "分割"));deleteButton.setButtonText(l("Delete clip", "删除片段"));
    loopButton.setButtonText(l("Loop song", "循环播放"));volumeLabel.setText(l("VOLUME", "音量"),dontSendNotification);panLabel.setText(l("PAN", "声像"),dontSendNotification);
    input.setTextWhenNothingSelected(l("Select recording input", "选择录音输入"));
    trackLabel.setText(l("SELECTED TRACK", "选定音轨")+"  "+String(selectedTrack+1),dontSendNotification);refresh();repaint();
}
void Studio::paint(Graphics& g){
    g.fillAll(bg);g.setColour(panel);g.fillRect(0,0,getWidth(),72);g.fillRect(0,72,getWidth(),64);g.fillRect(0,getHeight()-140,getWidth(),140);
    g.setColour(line);g.drawHorizontalLine(72,0,(float)getWidth());g.drawHorizontalLine(136,0,(float)getWidth());g.drawHorizontalLine(getHeight()-140,0,(float)getWidth());
    label(g,l("A small studio. A complete idea.", "小小工作站，记录完整灵感。"),{24,46,300,18},11,muted);
    label(g,l("RECORD TO SELECTED TRACK", "录到选定音轨"),{20,getHeight()-78,250,18},10,muted);
    label(g,l("Drag to move · edges to trim · click ruler to seek", "拖动移动 · 边缘裁剪 · 点击标尺定位"),{215,143,650,20},11,muted);
    label(g,l("VIEW", "视野"),{getWidth()-217,143,45,20},10,muted);
}
void Studio::resized(){
    auto w=getWidth();title.setBounds(24,12,170,34);
    newButton.setBounds(265,21,56,30);openButton.setBounds(327,21,56,30);saveButton.setBounds(389,21,56,30);
    importButton.setBounds(459,21,112,30);demoButton.setBounds(581,21,64,30);
    language.setBounds(w-438,21,110,30);settingsButton.setBounds(w-316,21,130,30);exportButton.setBounds(w-174,21,150,30);
    playButton.setBounds(24,88,90,34);stopButton.setBounds(122,88,74,34);recordButton.setBounds(208,88,100,34);
    clock.setBounds(w/2-85,83,170,40);loopButton.setBounds(w/2+100,91,135,28);
    addButton.setBounds(w-169,90,145,30);zoom.setBounds(w-176,141,152,25);
    viewport.setBounds(0,171,w,getHeight()-311);
    timeline.setSize(viewport.getMaximumVisibleWidth(),jmax(viewport.getHeight(),36+100*session.numTracks()));
    int bottom=getHeight()-130;
    trackLabel.setBounds(20,bottom,188,23);nameEditor.setBounds(20,bottom+29,180,27);
    volumeLabel.setBounds(232,bottom,80,20);volume.setBounds(230,bottom+27,230,30);
    panLabel.setBounds(480,bottom,80,20);pan.setBounds(478,bottom+27,200,30);
    splitButton.setBounds(w-290,bottom+26,100,30);deleteButton.setBounds(w-179,bottom+26,155,30);
    input.setBounds(20,getHeight()-57,265,28);message.setBounds(305,getHeight()-54,w-495,23);openSourceButton.setBounds(w-174,getHeight()-57,150,28);
}
void Studio::selectTrack(int n){
    selectedTrack=n;auto t=session.track(n);if(!t)return;
    nameEditor.setText(t->getName(),false);trackLabel.setText(l("SELECTED TRACK", "选定音轨")+"  "+String(n+1),dontSendNotification);
    volume.setValue(t->getVolumePlugin()->getVolumeDb(),dontSendNotification);pan.setValue(t->getVolumePlugin()->getPan(),dontSendNotification);
    timeline.repaint();
}
void Studio::resetView(){selected=nullptr;selectedTrack=0;timeline.clearCache();zoom.setRange(5,jmax(300.0,session.length()*1.2),1);zoom.setValue(jmax(30.0,session.length()*1.2));selectTrack(0);loopButton.setToggleState(false,dontSendNotification);refresh();}
void Studio::refresh(){
    auto& tc=session.edit->getTransport();bool recording=tc.isRecording();
    playButton.setButtonText(tc.isPlaying()?l("Pause", "暂停"):l("Play", "播放"));recordButton.setButtonText(recording?l("Finish take", "完成录音"):l("Record", "录音"));
    recordButton.setColour(TextButton::buttonColourId,recording?Colour(0xffae5459):Colour(0xff533238));
    for(auto b:{&newButton,&openButton,&saveButton,&importButton,&addButton,&demoButton,&settingsButton,&exportButton})b->setEnabled(!busy && !recording);
    playButton.setEnabled(!busy);stopButton.setEnabled(!busy);recordButton.setEnabled(!busy);
    splitButton.setEnabled(selected!=nullptr && !recording && !busy);deleteButton.setEnabled(selected!=nullptr && !recording && !busy);
    volume.setEnabled(!busy);pan.setEnabled(!busy);nameEditor.setEnabled(!busy && !recording);input.setEnabled(!busy && !recording);
    resized();timeline.repaint();
}
void Studio::timerCallback(){
    clock.setText(timeString(session.edit->getTransport().getPosition().inSeconds()),dontSendNotification);
    timeline.repaint();
    if(++ticks%20==0){updateInputs();refresh();}
    if(ticks%400==0 && !busy && !session.edit->getTransport().isRecording())te::EditFileOperations(*session.edit).save(false,true,false);
    if(render)status(l("Exporting WAV… ", "正在导出 WAV… ")+String((int)(render->getProgress()*100))+"%");
}
void Studio::togglePlay(){auto& tc=session.edit->getTransport();if(tc.isPlaying())tc.stop(false,false);else tc.play(false);refresh();}
void Studio::toggleRecord(){
    auto& tc=session.edit->getTransport();if(tc.isRecording()){session.stop();status(l("Take recorded", "录音已完成"));refresh();return;}
    if(input.getSelectedId()<1){status(l("Choose an input in Audio settings first", "请先在音频设置中启用输入通道"));return;}
    Component::SafePointer<Studio> safe(this);
    auto record=[safe]{if(!safe)return;auto id=safe->input.getSelectedId();
        if(id<1 || id>safe->inputIDs.size()){safe->status(safe->l("Choose an input in Audio settings first", "请先在音频设置中启用输入通道"));return;}
        auto r=safe->session.arm(safe->selectedTrack,safe->inputIDs[id-1]);
        if(r.failed()){safe->status(r.getErrorMessage());return;}
        safe->session.begin("Record audio");safe->session.edit->getTransport().record(false);
        safe->status(safe->l("Recording · press Stop to keep the take", "正在录音 · 点击停止保留录音"));safe->refresh();};
    if(!RuntimePermissions::isGranted(RuntimePermissions::recordAudio))RuntimePermissions::request(RuntimePermissions::recordAudio,[safe,record](bool granted){
        if(!safe)return;if(granted)record();else safe->status(safe->l("Microphone access denied. Enable it in System Settings.", "麦克风权限被拒绝，请在系统设置中启用。"));});
    else record();
}
void Studio::updateInputs(){
    StringArray ids,names;auto& dm=session.engine.getDeviceManager();
    for(int n=0;n<dm.getNumWaveInDevices();++n)if(auto d=dm.getWaveInDevice(n)){ids.add(d->getDeviceID());names.add(d->getName());}
    if(ids==inputIDs)return;
    String old=input.getSelectedId()>0?inputIDs[input.getSelectedId()-1]:String(); inputIDs=ids;input.clear(dontSendNotification);
    for(int n=0;n<names.size();++n)input.addItem(names[n],n+1);
    int ix=ids.indexOf(old);input.setSelectedId(ix>=0?ix+1:ids.isEmpty()?0:1,dontSendNotification);
}
void Studio::audioSettings(){
    DialogWindow::LaunchOptions options;options.dialogTitle=l("Audio settings", "音频设置");options.dialogBackgroundColour=panel;
    auto selector=new AudioDeviceSelectorComponent(session.engine.getDeviceManager().deviceManager,0,64,0,64,false,false,true,false);
    selector->setSize(480,510);options.content.setOwned(selector);options.launchAsync();
    status(l("Enable input channels to record. Monitoring is off to avoid feedback.", "录音前启用输入通道。输入监听已关闭，避免啸叫。"));
}
void Studio::chooseImport(){
    chooser=std::make_unique<FileChooser>(l("Import audio", "导入音频"),File::getSpecialLocation(File::userDocumentsDirectory),"*.wav;*.aif;*.aiff;*.mp3;*.flac;*.ogg;*.m4a");
    chooser->launchAsync(FileBrowserComponent::openMode|FileBrowserComponent::canSelectFiles|FileBrowserComponent::canSelectMultipleItems,
        [safe=Component::SafePointer<Studio>(this)](const FileChooser& fc){if(safe)for(auto& f:fc.getResults())safe->importFile(f);});
}
void Studio::importFile(const File& f,int target,double time){
    if(busy || session.edit->getTransport().isRecording())return;
    if(target<0) {auto t=session.track(selectedTrack);target=t && t->getClips().isEmpty()?selectedTrack:session.numTracks();}
    if(time<0)time=session.edit->getTransport().getPosition().inSeconds();
    if(target>=session.numTracks())session.addTrack(f.getFileNameWithoutExtension());
    String error;auto c=session.importAudio(f,target,time,error);
    if(!c){status(error);return;}selected=c;selectTrack(target);if(session.length()>viewSeconds){zoom.setRange(5,jmax(300.0,session.length()*1.2),1);zoom.setValue(session.length()*1.1);}
    status(l("Imported ", "已导入 ")+f.getFileName());refresh();
}
void Studio::chooseOpen(){
    chooser=std::make_unique<FileChooser>(l("Open Traka project", "打开 Traka 项目"),File::getSpecialLocation(File::userDocumentsDirectory),"*.traka");
    chooser->launchAsync(FileBrowserComponent::openMode|FileBrowserComponent::canSelectFiles,[safe=Component::SafePointer<Studio>(this)](const FileChooser& fc){
        if(!safe || fc.getResult()==File())return;safe->session.stop();te::EditFileOperations(*safe->session.edit).save(false,true,false);
        safe->selected=nullptr;auto r=safe->session.open(fc.getResult());safe->resetView();safe->status(r.wasOk()?safe->l("Project opened", "项目已打开"):r.getErrorMessage());});
}
void Studio::chooseSave(){
    if(session.named){auto r=session.save(session.project);status(r.wasOk()?l("Project saved", "项目已保存"):r.getErrorMessage());return;}
    chooser=std::make_unique<FileChooser>(l("Save Traka project", "保存 Traka 项目"),File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("My song.traka"),"*.traka");
    chooser->launchAsync(FileBrowserComponent::saveMode|FileBrowserComponent::warnAboutOverwriting,[safe=Component::SafePointer<Studio>(this)](const FileChooser& fc){
        if(!safe || fc.getResult()==File())return;auto r=safe->session.save(fc.getResult().withFileExtension("traka"));
        safe->status(r.wasOk()?safe->l("Saved project and its audio folder", "项目及音频文件夹已保存"):r.getErrorMessage());safe->refresh();});
}
void Studio::chooseExport(){
    if(session.length()<=0){status(l("Import or record audio before exporting", "请先导入或录制音频"));return;}
    chooser=std::make_unique<FileChooser>(l("Export WAV · stereo, 48 kHz, 24-bit", "导出 WAV · 立体声，48 kHz，24 位"),File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("Mix.wav"),"*.wav");
    chooser->launchAsync(FileBrowserComponent::saveMode|FileBrowserComponent::warnAboutOverwriting,[safe=Component::SafePointer<Studio>(this)](const FileChooser& fc){
        if(!safe || fc.getResult()==File())return;safe->session.stop();safe->busy=true;safe->refresh();
        auto target=fc.getResult().withFileExtension("wav");
        // Render to a sibling first so a failed/cancelled render cannot destroy an existing mix.
        auto temp=target.getSiblingFile(Uuid().toString()+".wav");
        safe->render=te::EditRenderer::render(safe->session.renderParameters(temp,safe->wav),[safe,temp,target](auto result){
            MessageManager::callAsync([safe,temp,target,ok=bool(result)] {
                if(!safe)return;safe->render.reset();safe->busy=false;
                if(ok && temp.moveFileTo(target))safe->status(safe->l("Exported: ", "已导出：")+target.getFullPathName());
                else {temp.deleteFile();safe->status(safe->l("Export failed. Check the output folder and audio files.", "导出失败，请检查输出目录和音频文件。"));}safe->refresh();
            });});});
}
void Studio::splitClip(){if(!selected)return;auto t=session.track(selectedTrack);if(!t)return;
    auto p=session.edit->getTransport().getPosition();auto range=selected->getPosition().time;
    if(p<=range.getStart() || p>=range.getEnd()){status(l("Place the playhead inside the selected clip", "请将播放头放在选定片段内"));return;}
    session.begin("Split clip");t->splitClip(*selected,p);refresh();}
void Studio::deleteClip(){if(!selected)return;session.begin("Delete clip");selected->removeFromParent();selected=nullptr;refresh();}
bool Studio::keyPressed(const KeyPress& k){
    if(busy)return false;
    if(k.getKeyCode()==KeyPress::spaceKey){togglePlay();return true;}
    if(k.getModifiers().isCommandDown()){
        if(k.getKeyCode()=='S'){chooseSave();return true;}
        if(k.getKeyCode()=='O'){chooseOpen();return true;}
        if(k.getKeyCode()=='Z' && !session.edit->getTransport().isRecording()){
            selected=nullptr;if(k.getModifiers().isShiftDown())session.edit->getUndoManager().redo();else session.edit->getUndoManager().undo();
            selectedTrack=jlimit(0,jmax(0,session.numTracks()-1),selectedTrack);selectTrack(selectedTrack);refresh();return true;}
    }
    if(k.getKeyCode()==KeyPress::backspaceKey || k.getKeyCode()==KeyPress::deleteKey){if(!session.edit->getTransport().isRecording())deleteClip();return true;}
    return false;
}
