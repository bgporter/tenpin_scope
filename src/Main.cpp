#include "MainComponent.h"
#include "model/appContext.h"
#include "model/persistentContext.h"
#include "utility/logger.h"

//==============================================================================
class TenpinScopeApplication final : public juce::JUCEApplication,
                                     public juce::Timer
{
public:
    //==============================================================================
    TenpinScopeApplication () {}

    // We inject these as compile definitions from the CMakeLists.txt
    // If you've enabled the juce header with `juce_generate_juce_header(<thisTarget>)`
    // you could `#include <JuceHeader.h>` and use `ProjectInfo::projectName` etc. instead.
    const juce::String getApplicationName () override { return JUCE_APPLICATION_NAME_STRING; }
    const juce::String getApplicationVersion () override { return JUCE_APPLICATION_VERSION_STRING; }
    bool moreThanOneInstanceAllowed () override { return true; }

    void timerCallback () override
    {
        PersistentContext pc { *appContext };
        if (pc.canUndo ())
        {
            pc.save (getConfigPath ());
            INFO_ ("Prefs file updated");
        }
    }

    //==============================================================================
    void initialise (const juce::String& commandLine) override
    {
        // This method is where you should put your application's initialisation code..
        TestSuite::runAllTests (commandLine);

        initializeLogger ();
        initializeAppContext ();
        mainWindow = std::make_unique<MainWindow> (getApplicationName (), *appContext);
        PersistentContext pc { *appContext };
        mainWindow->restoreWindowStateFromString (pc.windowState);
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

        PersistentContext pc { *appContext };
        pc.windowState = mainWindow->getWindowStateAsString ();
        pc.save (getConfigPath ());

        mainWindow = nullptr; // (deletes our window)
        appContext = nullptr;
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

    //==============================================================================
    /*
        This class implements the desktop window that contains an instance of
        our MainComponent class.
    */
    class MainWindow final : public juce::DocumentWindow
    {
    public:
        explicit MainWindow (juce::String name, AppContext& theAppContext)
        : DocumentWindow (name, juce::Desktop::getInstance ().getDefaultLookAndFeel ().findColour (backgroundColourId),
                          allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent (theAppContext), true);

#if JUCE_IOS || JUCE_ANDROID
            setFullScreen (true);
#else
            setResizable (true, true);
            centreWithSize (getWidth (), getHeight ());
#endif

            setVisible (true);
        }

        void closeButtonPressed () override
        {
            // This is called when the user tries to close this window. Here, we'll just
            // ask the app to quit when this happens, but you can change this to do
            // whatever you need.
            getInstance ()->systemRequestedQuit ();
        }

        /* Note: Be careful if you override any DocumentWindow methods - the base
           class uses a lot of them, so by overriding you might break its functionality.
           It's best to do all your work in your content component instead, but if
           you really have to override any DocumentWindow methods, make sure your
           subclass also calls the superclass's method.
        */

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
    std::unique_ptr<AppContext> appContext;
    juce::UndoManager undoManager;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (TenpinScopeApplication)
