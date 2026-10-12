// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 hsduren and Traka contributors
#pragma once
#include <JuceHeader.h>
#include <tracktion_engine/tracktion_engine.h>
namespace te = tracktion;
using namespace juce;
inline te::TimePosition tp(double s) { return te::TimePosition::fromSeconds(s); }
inline te::TimeDuration td(double s) { return te::TimeDuration::fromSeconds(s); }
class TrakaBehaviour : public te::EngineBehaviour {
public:
    bool shouldOpenAudioInputByDefault() override { return false; }
    int getNumberOfCPUsToUseForAudio() override { return 4; }
    File getDefaultFolderForAudioRecordings(te::Edit& e) override {
        auto d = e.editFileRetriever().getSiblingFile(e.editFileRetriever().getFileName() + ".media");
        d.createDirectory(); return d;
    }
};
class Session {
public:
    te::Engine engine { "Traka", nullptr, std::make_unique<TrakaBehaviour>() };
    std::unique_ptr<te::Edit> edit;
    File project;
    bool named = false;
    Session();
    ~Session();
    void newProject();
    Result open(const File&);
    Result save(const File&);
    te::AudioTrack* addTrack(const String& name = {});
    te::WaveAudioClip::Ptr importAudio(const File&, int trackIndex, double start, String& error);
    te::AudioTrack* track(int index) const;
    int numTracks() const;
    double length() const;
    Result arm(int index, const String& inputID);
    void disarm();
    void begin(const String& transaction);
    void stop();
    te::Renderer::Parameters renderParameters(const File&, WavAudioFormat&);
    Result makeDemo();
};
int runSelfTest();
