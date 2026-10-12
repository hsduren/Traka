// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 hsduren and Traka contributors
#include "Studio.h"
#include <iostream>
void Studio::runSmoke(const File& output) {
    output.createDirectory();
    auto log=output.getChildFile("ui-smoke.txt");log.replaceWithText("Traka VM UI workflow\n");
    auto check=[log](bool ok,const String& name){log.appendText((ok?"PASS ":"FAIL ")+name+"\n");return ok;};
    auto capture=[this,output](const String& name){
        PNGImageFormat png; auto stream=output.getChildFile(name+".png").createOutputStream();
        if(stream){stream->setPosition(0);stream->truncate();png.writeImageToStream(createComponentSnapshot(getLocalBounds(),true,1.0f),*stream);}
    };
    demoButton.onClick();
    check(session.numTracks()==4 && session.length()==12,"demo has three clips and recording track");
    selectTrack(0);selected=session.track(0)->getClips()[0];
    session.begin("UI smoke move");selected->setStart(tp(1),false,true);
    check(selected->getPosition().time.getStart()==tp(1),"timeline move uses audio model");
    session.edit->getTransport().setPosition(tp(5));splitButton.onClick();
    check(session.track(0)->getClips().size()==2,"split action");
    volume.setValue(-6,sendNotificationSync);check(std::abs(session.track(0)->getVolumePlugin()->getVolumeDb()+6)<0.01,"volume control");
    session.edit->getTransport().setPosition(tp(0));playButton.onClick();
    Component::SafePointer<Studio> safe(this);
    Timer::callAfterDelay(1000,[safe,output,check,capture]{
        if(!safe)return;
        check(safe->session.edit->getTransport().isPlaying() && safe->session.edit->getTransport().getPosition().inSeconds()>0.1,"transport advances in guest");
        safe->stopButton.onClick();
        check(!safe->session.edit->getTransport().isPlaying(),"stop action");
        safe->selected=nullptr;
        auto project=output.getChildFile("VM song.traka");
        check(safe->session.save(project).wasOk(),"save project");
        check(safe->session.open(project).wasOk(),"reopen project");safe->resetView();
        safe->language.setSelectedId(1,sendNotificationSync);capture("studio-en");
        safe->language.setSelectedId(2,sendNotificationSync);capture("studio-zh");
        check(safe->newButton.getButtonText()==String::fromUTF8("新建"),"live language switch");
        safe->status(safe->l("VM validation: exporting", "VM 验证：正在导出"));safe->busy=true;safe->refresh();
        auto mix=output.getChildFile("VM mix.wav");
        safe->render=te::EditRenderer::render(safe->session.renderParameters(mix,safe->wav),[safe,mix,output,check,capture](auto result){
            MessageManager::callAsync([safe,mix,output,check,capture,ok=bool(result)]{
                if(!safe)return;safe->render.reset();safe->busy=false;safe->refresh();
                check(ok && mix.existsAsFile(),"asynchronous export finishes");
                std::unique_ptr<AudioFormatReader> reader;
                if(auto stream=mix.createInputStream())reader.reset(safe->wav.createReaderFor(stream.release(),true));
                check(reader && reader->sampleRate==48000 && reader->bitsPerSample==24 && reader->numChannels==2,"WAV export format");
                if(reader){AudioBuffer<float>b(2,(int)reader->lengthInSamples);reader->read(&b,0,b.getNumSamples(),0,true,true);check(b.getMagnitude(0,b.getNumSamples())>0.01f,"export contains audio");}
                safe->status(safe->l("VM validation complete", "VM 验证完成"));capture("studio-final");
                safe->openSourceButton.onClick();
                bool hasNotices=false;
                for(int n=0;n<Desktop::getInstance().getNumComponents();++n) {
                    if(auto dialog=dynamic_cast<DialogWindow*>(Desktop::getInstance().getComponent(n))) {
                        auto body=dialog->getContentComponent();
                        if(body)if(auto editor=dynamic_cast<TextEditor*>(body->getChildComponent(0)))
                            hasNotices=editor->getText().contains(TRAKA_SOURCE_URL) && editor->getText().contains("GNU AFFERO GENERAL PUBLIC LICENSE");
                        if(hasNotices){
                            PNGImageFormat png;auto stream=output.getChildFile("license-vm.png").createOutputStream();
                            if(stream){stream->setPosition(0);stream->truncate();png.writeImageToStream(body->createComponentSnapshot(body->getLocalBounds(),true,1.0f),*stream);}
                            dialog->exitModalState(0);break;
                        }
                    }
                }
                check(hasNotices,"license dialog contains source URL and full license text");
                auto failed=output.getChildFile("ui-smoke.txt").loadFileAsString().contains("FAIL");
                JUCEApplication::getInstance()->setApplicationReturnValue(failed?1:0);JUCEApplication::getInstance()->quit();
            });
        });
    });
}
