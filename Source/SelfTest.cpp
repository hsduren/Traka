// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 hsduren and Traka contributors
#include "Session.h"
#include <iostream>
namespace {
File renderForTest(te::Renderer::Parameters params) {
    te::Edit::ScopedRenderStatus status(*params.edit, true);
    auto task = te::render_utils::createRenderTask(params, "Traka test", nullptr, nullptr);
    if (!task) return {};
    while (task->runJob() == ThreadPoolJob::jobNeedsRunningAgain) {}
    if (task->errorMessage.isNotEmpty()) { std::cerr << task->errorMessage << std::endl; return {}; }
    return params.destFile;
}
}
int runSelfTest(){
    std::cout << "Traka audio workflow test\n";
    Session s;
    auto dir=File::getSpecialLocation(File::tempDirectory).getChildFile("Traka-test-"+Uuid().toString());dir.createDirectory();
    int failures=0;
    auto check=[&](bool ok,const char* what){std::cout<<(ok?"PASS ":"FAIL ")<<what<<std::endl;if(!ok)++failures;};
    auto notices=File::getSpecialLocation(File::currentApplicationFile).getChildFile("Contents/Resources/OpenSource");
    check(notices.getChildFile("AGPL-3.0.txt").loadFileAsString().contains("GNU AFFERO GENERAL PUBLIC LICENSE"),"app bundles full AGPL license");
    check(notices.getChildFile("SOURCE.txt").loadFileAsString().contains("https://github.com/hsduren/Traka/tree/v" TRAKA_VERSION),"app bundles source URL for its exact version");
    check(s.makeDemo().wasOk(),"create independently generated demo audio");
    check(s.numTracks()==4 && s.length()>11.99,"three audio tracks plus empty recording track");
    check(s.arm(3,"unavailable-test-input").failed(),"unavailable recording input rejected");
    te::Clip::Ptr c=s.track(0)->getClips()[0];
    s.begin("Move test");c->setStart(tp(1),false,true);
    check(std::abs(c->getPosition().time.getStart().inSeconds()-1)<0.001,"move clip");
    s.edit->getUndoManager().undo();check(c->getPosition().time.getStart().inSeconds()==0,"undo movement");
    s.edit->getUndoManager().redo();check(c->getPosition().time.getStart().inSeconds()==1,"redo movement");
    s.begin("Trim test");c->setStart(tp(2),true,false);c->setEnd(tp(6),true);
    check(std::abs(c->getPosition().offset.inSeconds()-1)<0.001 && c->getPosition().time.getEnd().inSeconds()==6,"non-destructive trim preserves source offset");
    s.begin("Split test");s.track(0)->splitClip(*c,tp(4));check(s.track(0)->getClips().size()==2,"split clip");
    s.track(1)->getVolumePlugin()->setVolumeDb(-9);s.track(2)->setMute(true);
    c=nullptr;
    auto project=dir.getChildFile("Saved.traka");check(s.save(project).wasOk(),"save project with copied audio");
    check(s.open(project).wasOk(),"reopen project");
    check(s.track(0)->getClips().size()==2 && s.track(2)->isMuted(false) && std::abs(s.track(1)->getVolumePlugin()->getVolumeDb()+9)<0.01,"persist edits, mute and volume");
    auto relocated=dir.getChildFile("Relocated");relocated.createDirectory();
    project.copyFileTo(relocated.getChildFile("Saved.traka"));dir.getChildFile("Saved.traka.media").copyDirectoryTo(relocated.getChildFile("Saved.traka.media"));
    check(s.open(relocated.getChildFile("Saved.traka")).wasOk(),"portable relative media references");
    auto missing=dir.getChildFile("broken.traka");missing.replaceWithText("garbage");
    auto before=s.project;check(s.open(missing).failed() && s.project==before,"invalid project does not replace session");
    WavAudioFormat wav;auto out=dir.getChildFile("mix.wav");
    auto params=s.renderParameters(out,wav);
    auto rendered=renderForTest(params);
    check(rendered.existsAsFile(),"render mixed WAV");
    std::unique_ptr<AudioFormatReader> reader; if(auto stream=out.createInputStream())reader.reset(wav.createReaderFor(stream.release(),true));
    check(reader && reader->sampleRate==48000 && reader->bitsPerSample==24 && reader->numChannels==2,"48 kHz / 24-bit / stereo format");
    if(reader){AudioBuffer<float> b(2,(int)reader->lengthInSamples);reader->read(&b,0,b.getNumSamples(),0,true,true);check(b.getMagnitude(0,b.getNumSamples())>0.01f,"mix contains audible samples");check(std::abs(reader->lengthInSamples/reader->sampleRate-s.length())<0.05,"correct export duration");}
    auto mutedFile=dir.getChildFile("muted.wav");for(auto t:te::getAudioTracks(*s.edit))t->setMute(true);
    auto mp=s.renderParameters(mutedFile,wav);mp.checkNodesForAudio=false;
    auto mutedRender=renderForTest(mp);
    std::unique_ptr<AudioFormatReader> mr; if(auto stream=mutedFile.createInputStream())mr.reset(wav.createReaderFor(stream.release(),true));
    if(mr){AudioBuffer<float>b(2,(int)mr->lengthInSamples);mr->read(&b,0,b.getNumSamples(),0,true,true);check(b.getMagnitude(0,b.getNumSamples())<0.0001f,"muted tracks excluded from export");}else check(false,"muted export readable");
    s.stop();reader.reset();mr.reset();c=nullptr;s.edit.reset();dir.deleteRecursively();
    std::cout<<"Failures: "<<failures<<std::endl;return failures?1:0;
}
