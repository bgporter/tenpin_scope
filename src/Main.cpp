/*
 MIT License

 Copyright (c) 2026 Brett g Porter

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include "MainComponent.h"
#include "controller/midiController.h"
#include "mainWindow.h"
#include "model/appContext.h"
#include "model/persistentContext.h"
#include "utility/logger.h"
#include "view/lookAndFeel.h"

//==============================================================================
class _10PinScopeApplication final : public juce::JUCEApplication,
                                     public juce::Timer
{
public:
    //==============================================================================
    _10PinScopeApplication () {}

    // We inject these as compile definitions from the CMakeLists.txt
    // If you've enabled the juce header with `juce_generate_juce_header(<thisTarget>)`
    // you could `#include <JuceHeader.h>` and use `ProjectInfo::projectName` etc. instead.
    const juce::String getApplicationName () override { return JUCE_APPLICATION_NAME_STRING; }
    const juce::String getApplicationVersion () override { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed () override { return true; }

    void timerCallback () override
    {
        PersistentContext pc { *appContext };
        if (pc.canUndo () && !pc.dragging)
        {
            pc.save (getConfigPath ());
            DBG ("Saved prefs file: " << getConfigPath ().getFullPathName ());
            INFO_ ("Prefs file updated");
        }
    }

    //==============================================================================
    void initialise (const juce::String& commandLine) override
    {
        // This method is where you should put your application's initialisation code..
        initializeLogger ();

        LogWriter::enableLogging (false);
        TestSuite::runAllTests (commandLine);
        LogWriter::enableLogging (true);

        initializeAppContext ();
        PersistentContext pc { *appContext };
        // Capture before MainWindow construction overwrites pc.windowState via
        // MainWindow::resized() / moved() callbacks.
        const juce::String savedWindowState = pc.windowState;

#if JUCE_DEBUG
        // in debug builds, we want to log everything.
        LogWriter::setLogLevel (LogWriter::Level::trace);
#else
        // in release builds, we want to log at the level specified in the prefs file.
        LogWriter::setLogLevel (pc.logLevel);
#endif

        lookAndFeel = std::make_unique<_10PinLookAndFeel> (*appContext);
        juce::LookAndFeel::setDefaultLookAndFeel (lookAndFeel.get ());

        midiController = std::make_unique<MidiController> (getApplicationName (), *appContext);

        mainWindow = std::make_unique<MainWindow> (getApplicationName (), *appContext);
        mainWindow->restoreWindowStateFromString (savedWindowState);
        // Each timer callback we'll see if the prefs file needs to be flushed to disk
        startTimer (1000);
    }

    juce::File getBaseDataPath ()
    {
        juce::String appName { getApplicationName () };
        juce::File basePath {
            juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile (appName)
        };
        if (!basePath.exists ())
        {
            auto ok = basePath.createDirectory ();
            if (!ok)
            {
                jassertfalse;
                return {};
                // !!! handle properly
            }
        }
        return basePath;
    }

    juce::File getLogPath ()
    {
        auto base { getBaseDataPath () };
        if (base == juce::File {})
            return {};
        return base.getChildFile (getApplicationName () + ".log");
    }

    void initializeLogger ()
    {
        LogWriter::init (getLogPath ());
        INFO_ ("==============================================");
        INFO_ ({
            {     "msg",                                                       "STARTING" },
            {    "time", juce::Time::getCurrentTime ().formatted ("%d %b %Y %H:%M:%S %Z") },
            { "version",                                         getApplicationVersion () }
        });
    }

    juce::File getConfigPath ()
    {
        auto base { getBaseDataPath () };
        if (base == juce::File {})
            return {};
        return base.getChildFile (getApplicationName () + ".prefs");
    }

    void initializeAppContext ()
    {
        appContext = std::make_unique<AppContext> (getConfigPath ());
        appContext->setUndoManager (&undoManager);
        appContext->clearUndoHistory ();
    }

    void shutdown () override
    {
        // Add your application's shutdown code here..
        INFO_ ({
            {  "msg",                                                       "SHUTDOWN" },
            { "time", juce::Time::getCurrentTime ().formatted ("%d %b %Y %H:%M:%S %Z") }
        });
        LogWriter::shutdown ();
        // stop the timer that keeps the prefs file updated...
        stopTimer ();
        // ...and call the timer callback to ensure the prefs file is saved if needed.
        timerCallback ();

        midiController = nullptr;

        mainWindow = nullptr; // (deletes our window)
        juce::LookAndFeel::setDefaultLookAndFeel (nullptr);
        lookAndFeel = nullptr;
        appContext  = nullptr;
    }

    //==============================================================================
    void systemRequestedQuit () override
    {
        // This is called when the app is being asked to quit: you can ignore this
        // request and let the app carry on running, or call quit() to allow the app to close.
        quit ();
    }

    void anotherInstanceStarted (const juce::String& commandLine) override
    {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
        juce::ignoreUnused (commandLine);
    }

private:
    juce::UndoManager undoManager;
    std::unique_ptr<MainWindow> mainWindow;
    std::unique_ptr<AppContext> appContext;
    std::unique_ptr<MidiController> midiController;
    std::unique_ptr<_10PinLookAndFeel> lookAndFeel;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (_10PinScopeApplication)
