// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 hsduren and Traka contributors
#pragma once
#include "Session.h"
class Studio;
class Timeline : public Component, public FileDragAndDropTarget {
public:
    explicit Timeline(Studio&);
    void paint(Graphics&) override;
    void mouseDown(const MouseEvent&) override;
    void mouseDrag(const MouseEvent&) override;
    void mouseUp(const MouseEvent&) override;
    bool isInterestedInFileDrag(const StringArray&) override { return true; }
    void filesDropped(const StringArray&, int, int) override;
    double secondsAt(float x) const;
    float xAt(double s) const;
    int rowAt(int y) const { return (y - 36) / 100; }
    void clearCache() { thumbnails.clear(); }
    static constexpr int header = 208;
private:
    Studio& studio;
    AudioFormatManager formats;
    AudioThumbnailCache cache {32};
    std::map<String, std::unique_ptr<AudioThumbnail>> thumbnails;
    te::Clip::Ptr dragged;
    te::ClipPosition original;
    int dragMode = 0;
};
class Studio : public Component, private Timer {
public:
    Studio();
    ~Studio() override;
    void paint(Graphics&) override;
    void resized() override;
    bool keyPressed(const KeyPress&) override;
    void importFile(const File&, int target = -1, double time = -1);
    void selectTrack(int);
    void refresh();
    void status(const String&);
    void runSmoke(const File& output);
    String l(const char* en, const char* zh) const;
    Session session;
    te::Clip::Ptr selected;
    int selectedTrack = 0;
    double viewSeconds = 30;
    bool chinese = false;
private:
    friend class Timeline;
    LookAndFeel_V4 theme;
    Timeline timeline {*this};
    Viewport viewport;
    TextButton newButton, openButton, saveButton, importButton, addButton, demoButton,
        settingsButton, openSourceButton, exportButton, playButton, stopButton, recordButton, splitButton, deleteButton;
    ToggleButton loopButton;
    Label title, clock, message, trackLabel, volumeLabel, panLabel;
    TextEditor nameEditor;
    Slider volume, pan, zoom;
    ComboBox input, language;
    std::unique_ptr<FileChooser> chooser;
    WavAudioFormat wav;
    std::shared_ptr<te::EditRenderer::Handle> render;
    String messageText;
    StringArray inputIDs;
    int ticks = 0;
    bool busy = false;
    void timerCallback() override;
    void chooseOpen();
    void chooseSave();
    void chooseImport();
    void chooseExport();
    void audioSettings();
    void toggleRecord();
    void togglePlay();
    void deleteClip();
    void splitClip();
    void updateInputs();
    void updateLanguage();
    void resetView();
};
