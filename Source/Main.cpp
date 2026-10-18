// SPDX-License-Identifier: AGPL-3.0-or-later
// Copyright (C) 2026 hsduren and Traka contributors
#include "Studio.h"
void trakaRefreshDockIcon();
void trakaApplyDarkAppearance();
class TrakaApp : public JUCEApplication {
public:
    const String getApplicationName() override {return "Traka";}
    const String getApplicationVersion() override {return TRAKA_VERSION;}
    bool moreThanOneInstanceAllowed() override {return true;}
    void initialise(const String& args) override {
        if(args.contains("--self-test")){setApplicationReturnValue(runSelfTest());quit();return;}
        trakaApplyDarkAppearance();
        window=std::make_unique<Window>();
        trakaRefreshDockIcon();
        Timer::callAfterDelay(500, [] { trakaRefreshDockIcon(); });
        if(args.contains("--ui-smoke")) {
            auto output=File::getSpecialLocation(File::userDocumentsDirectory).getChildFile("Traka-P0-QA");
            auto studio=dynamic_cast<Studio*>(window->getContentComponent());
            Timer::callAfterDelay(500,[studio,output]{studio->runSmoke(output);});
        }
    }
    void shutdown() override {window.reset();}
    void systemRequestedQuit() override {quit();}
    class Window : public DocumentWindow {
    public:
        Window():DocumentWindow(String::fromUTF8("Traka · P0"),Colour(0xff14171c),DocumentWindow::allButtons){
            setUsingNativeTitleBar(true);setContentOwned(new Studio(),true);setResizable(true,true);
            setResizeLimits(1100,640,2400,1800);centreWithSize(getWidth(),getHeight());setVisible(true);
        }
        void closeButtonPressed() override {JUCEApplication::getInstance()->systemRequestedQuit();}
    };
    std::unique_ptr<Window> window;
};
START_JUCE_APPLICATION(TrakaApp)
