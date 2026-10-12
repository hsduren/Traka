// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 hsduren and Traka contributors
#include "Session.h"
Session::Session() { newProject(); }
Session::~Session() { stop(); if (edit) te::EditFileOperations(*edit).save(false, true, false); }
void Session::newProject() {
    stop(); edit.reset(); named = false;
    auto folder = File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("Traka/Sessions")
        .getChildFile(Uuid().toString());
    folder.createDirectory(); project = folder.getChildFile("Untitled.traka");
    edit = te::createEmptyEdit(engine, project);
    edit->editFileRetriever = [this] { return project; };
    edit->playInStopEnabled = true;
    edit->ensureNumberOfAudioTracks(1); track(0)->setName("Audio 01");
    te::EditFileOperations(*edit).save(false, true, false);
    edit->getUndoManager().clearUndoHistory();
}
void Session::stop() { if (edit) edit->getTransport().stop(false, false); }
Result Session::open(const File& file) {
    auto xml = XmlDocument::parse(file);
    if (!xml || !xml->hasTagName("EDIT")) return Result::fail("Invalid Traka project");
    auto candidate = te::loadEditFromFile(engine, file);
    if (!candidate) return Result::fail("Could not open project");
    stop(); edit = std::move(candidate); project = file; named = true;
    edit->editFileRetriever = [this] { return project; };
    edit->playInStopEnabled = true;
    disarm(); edit->getUndoManager().clearUndoHistory();
    StringArray missing;
    for (auto t : te::getAudioTracks(*edit)) for (auto c : t->getClips())
        if (auto w = dynamic_cast<te::WaveAudioClip*>(c))
            if (!w->getSourceFileReference().getFile().existsAsFile()) missing.add(w->getName());
    return missing.isEmpty() ? Result::ok() : Result::fail("Missing audio: " + missing.joinIntoString(", "));
}
Result Session::save(const File& target) {
    stop();
    auto media = target.getSiblingFile(target.getFileName() + ".media");
    if (auto r = media.createDirectory(); r.failed()) return r;
    struct Remap { te::Clip::Ptr clip; File source, dest; };
    std::vector<Remap> mappings;
    std::map<String, File> copied;
    for (auto t : te::getAudioTracks(*edit)) for (auto c : t->getClips()) {
        auto src = c->getSourceFileReference().getFile();
        if (!src.existsAsFile()) return Result::fail("Missing audio: " + c->getName());
        File dst;
        if (src.getParentDirectory() == media) dst = src;
        else if (copied.count(src.getFullPathName())) dst = copied.at(src.getFullPathName());
        else {
            dst = media.getNonexistentChildFile(src.getFileNameWithoutExtension(), src.getFileExtension());
            if (!src.copyFileTo(dst)) return Result::fail("Could not copy audio: " + src.getFileName());
            copied[src.getFullPathName()] = dst;
        }
        mappings.push_back({ c, src, dst });
    }
    auto old = project; project = target;
    for (auto& m : mappings) m.clip->getSourceFileReference().source.setValue(m.dest.getRelativePathFrom(project.getParentDirectory()), nullptr);
    bool ok = false;
    te::EditFileOperations(*edit).save(false, true, false, [&ok](bool b) { ok = b; });
    if (!ok) {
        project = old;
        for (auto& m : mappings) m.clip->getSourceFileReference().source.setValue(m.source.getRelativePathFrom(project.getParentDirectory()), nullptr);
        return Result::fail("Could not write project");
    }
    named = true; return Result::ok();
}
void Session::begin(const String& name) { edit->getUndoManager().beginNewTransaction(name); }
int Session::numTracks() const { return te::getAudioTracks(*edit).size(); }
te::AudioTrack* Session::track(int i) const { auto ts = te::getAudioTracks(*edit); return isPositiveAndBelow(i, ts.size()) ? ts[i] : nullptr; }
te::AudioTrack* Session::addTrack(const String& name) {
    begin("Add audio track"); auto count = numTracks(); edit->ensureNumberOfAudioTracks(count + 1);
    auto t = track(count); t->setName(name.isEmpty() ? "Audio " + String(count + 1).paddedLeft('0',2) : name); return t;
}
double Session::length() const {
    double end = 0;
    for (auto t : te::getAudioTracks(*edit)) for (auto c : t->getClips()) end = jmax(end, c->getPosition().time.getEnd().inSeconds());
    return end;
}
te::WaveAudioClip::Ptr Session::importAudio(const File& file, int i, double start, String& error) {
    te::AudioFile af(engine, file);
    if (!af.isValid() || af.getLength() <= 0) { error = "Unsupported or empty audio file: " + file.getFileName(); return {}; }
    auto media = project.getSiblingFile(project.getFileName() + ".media");
    if (auto r = media.createDirectory(); r.failed()) { error = r.getErrorMessage(); return {}; }
    auto dest = media.getNonexistentChildFile(file.getFileNameWithoutExtension(), file.getFileExtension());
    if (!file.copyFileTo(dest)) { error = "Could not copy audio"; return {}; }
    begin("Import audio"); auto t = track(i); if (!t) t = addTrack();
    auto c = t->insertWaveClip(file.getFileNameWithoutExtension(), dest, {{tp(start), tp(start + af.getLength())}, td(0)}, false);
    if (c) { c->setAutoTempo(false); c->getSourceFileReference().setToFile(dest, te::SourceFileReference::PathStyle::alwaysRelative, false); }
    else error = "Could not create audio clip";
    return c;
}
void Session::disarm() {
    if (!edit) return;
    for (auto instance : edit->getAllInputDevices()) for (auto t : te::getAudioTracks(*edit))
        instance->setRecordingEnabled(t->itemID, false);
}
Result Session::arm(int i, const String& inputID) {
    auto t = track(i); if (!t) return Result::fail("Select an audio track");
    auto& dm = engine.getDeviceManager();
    te::WaveInputDevice* input = nullptr;
    for (int n = 0; n < dm.getNumWaveInDevices(); ++n) if (auto d = dm.getWaveInDevice(n)) {
        if (d->getDeviceID() == inputID) input = d;
        d->setMonitorMode(te::InputDevice::MonitorMode::off);
    }
    if (!input) return Result::fail("Select an input in Audio settings");
    input->setEnabled(true); edit->getTransport().ensureContextAllocated(); disarm();
    for (auto instance : edit->getAllInputDevices()) if (&instance->getInputDevice() == input) {
        auto result = instance->setTarget(t->itemID, true, &edit->getUndoManager(), 0);
        if (!result) return Result::fail(String(result.error()));
        instance->setRecordingEnabled(t->itemID, true); edit->restartPlayback(); return Result::ok();
    }
    return Result::fail("Input is not available. Enable its channels in Audio settings.");
}
te::Renderer::Parameters Session::renderParameters(const File& f, WavAudioFormat& format) {
    te::Renderer::Parameters p(*edit); p.destFile = f; p.audioFormat = &format;
    p.tracksToDo = te::toBitSet(te::getAllTracks(*edit));
    p.time = {tp(0), tp(length())}; p.bitDepth = 24; p.sampleRateForAudio = 48000;
    p.canRenderInMono = false; p.useMasterPlugins = true; return p;
}
Result Session::makeDemo() {
    newProject();
    auto folder = project.getParentDirectory();
    const char* names[] = {"Warm keys", "Low tide", "Pulse"};
    for (int n = 0; n < 3; ++n) {
        auto f = folder.getChildFile(String(names[n]) + ".wav");
        WavAudioFormat format; std::unique_ptr<OutputStream> stream = f.createOutputStream();
        auto writer = format.createWriterFor(stream, AudioFormatWriterOptions{}.withSampleRate(48000).withNumChannels(2).withBitsPerSample(24));
        if (!writer) return Result::fail("Could not create demo audio");
        AudioBuffer<float> b(2, 48000 * 12); b.clear();
        for (int s = 0; s < b.getNumSamples(); ++s) {
            double time = s / 48000.0, phase = std::fmod(time, n == 2 ? 0.5 : 2.0);
            double value = 0;
            if (n == 0) {
                double notes[] = {261.63, 329.63, 392.0};
                for (auto hz : notes) value += std::sin(MathConstants<double>::twoPi * hz * time) * std::exp(-phase * 2.2) * 0.055;
            } else if (n == 1) value = std::sin(MathConstants<double>::twoPi * 65.406 * time) * std::exp(-phase * 3) * 0.16;
            else value = std::sin(MathConstants<double>::twoPi * (55 * phase + 9 * (1 - std::exp(-phase * 35)))) * std::exp(-phase * 28) * 0.28;
            value *= jmin(1.0, time * 80) * jmin(1.0, (12 - time) * 20);
            b.setSample(0, s, (float)value); b.setSample(1, s, (float)value);
        }
        writer->writeFromAudioSampleBuffer(b, 0, b.getNumSamples()); writer.reset();
        if (n > 0) addTrack(names[n]); else track(0)->setName(names[n]);
        String error; if (!importAudio(f, n, 0, error)) return Result::fail(error);
    }
    addTrack("Voice"); edit->getTransport().setLoopRange({tp(0),tp(12)});
    edit->getUndoManager().clearUndoHistory(); return Result::ok();
}
