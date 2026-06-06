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

#include "mainWindow.h"
#include "MainComponent.h"
#include "model/commands.h"
#include "model/persistentContext.h"
#include <juce_core/juce_core.h>

namespace
{
enum class MenuIndex
{
    file = 0,
    edit,
    view,
    options,
    help
};

} // namespace

MainWindow::MainWindow (juce::String name, AppContext& theAppContext)
: DocumentWindow (name, juce::Colours::grey, juce::DocumentWindow::allButtons)
, appContext (theAppContext)
{
    setUsingNativeTitleBar (true);
    auto mainComponent = std::make_unique<MainComponent> (appContext);
    setContentOwned (mainComponent.release (), true);

#if JUCE_IOS || JUCE_ANDROID
    setFullScreen (true);
#else
    setResizable (true, true);
    centreWithSize (getWidth (), getHeight ());
#endif

    setVisible (true);

    setApplicationCommandManagerToWatch (cmdManager);
    cmdManager->registerAllCommandsForTarget (this);

    juce::KeyPressMappingSet* mappings { cmdManager->getKeyMappings () };
    mappings->resetToDefaultMappings ();
    // TODO: reload user mappings.

#if JUCE_MAC
    setMacMainMenu (this);
#else
    setMenuBar (this);
#endif
}

MainWindow::~MainWindow ()
{
#if JUCE_MAC
    setMacMainMenu (nullptr);
#else
    setMenuBar (nullptr);
#endif
}

void MainWindow::closeButtonPressed ()
{
    // This is called when the user tries to close this window. Here, we'll just
    // ask the app to quit when this happens, but you can change this to do
    // whatever you need.
    auto* app = juce::JUCEApplication::getInstance ();
    if (app != nullptr)
        app->systemRequestedQuit ();
}

bool MainWindow::okayToClose ()
{
    // TODO: add actual logic...
    return true;
}

void MainWindow::moved ()
{
    PersistentContext pc { appContext };
    pc.windowState = getWindowStateAsString ();
    DocumentWindow::moved ();
}

void MainWindow::resized ()
{
    PersistentContext pc { appContext };
    pc.windowState = getWindowStateAsString ();
    DocumentWindow::resized ();
}
juce::StringArray MainWindow::getMenuBarNames ()
{
    return StringArray { TRANS ("File"), TRANS ("Edit"), TRANS ("View"), TRANS ("Options"), TRANS ("Help") };
}
juce::PopupMenu MainWindow::getMenuForIndex (int topLevelMenuIndex, const juce::String& /*menuName*/)
{
    PopupMenu menu;

    switch (static_cast<MenuIndex> (topLevelMenuIndex))
    {
        case MenuIndex::file:
            createFileMenu (menu);
            break;
        case MenuIndex::edit:
            createEditMenu (menu);
            break;
        case MenuIndex::view:
            createViewMenu (menu);
            break;
        case MenuIndex::options:
            createOptionsMenu (menu);
            break;
        case MenuIndex::help:
            createHelpMenu (menu);
            break;
    }
    return menu;
}
void MainWindow::menuItemSelected (int menuItemID, int topLevelMenuIndex)
{
    // TODO: recent file menu support
    juce::ignoreUnused (menuItemID);
    juce::ignoreUnused (topLevelMenuIndex);
}

ApplicationCommandTarget* MainWindow::getNextCommandTarget ()
{
    return findFirstTargetParentComponent ();
}

void MainWindow::getAllCommands (juce::Array<juce::CommandID>& commands)
{
    commands.addArray ({ AppCommands::incTextHeight, AppCommands::decTextHeight });
}

void MainWindow::getCommandInfo (juce::CommandID commandID, juce::ApplicationCommandInfo& result)
{
    PersistentContext pc { appContext };
    const float textHeight = pc.eventViewContext.textHeight.get ();

    switch (commandID)
    {
        case AppCommands::incTextHeight:
            result.setInfo ("Larger Text", "Increase text size", "View", 0);
            result.setActive (textHeight < 24.f);
            result.addDefaultKeypress ('=', juce::ModifierKeys::commandModifier);
            break;
        case AppCommands::decTextHeight:
            result.setInfo ("Smaller Text", "Decrease text size", "View", 0);
            result.setActive (textHeight > 8.f);
            result.addDefaultKeypress ('-', juce::ModifierKeys::commandModifier);
            break;
        default:
            break;
    }
}

bool MainWindow::perform (const juce::ApplicationCommandTarget::InvocationInfo& info)
{
    PersistentContext pc { appContext };
    const float current = pc.eventViewContext.textHeight.get ();

    switch (info.commandID)
    {
        case AppCommands::incTextHeight:
            pc.eventViewContext.textHeight = current + 1.f;
            return true;
        case AppCommands::decTextHeight:
            pc.eventViewContext.textHeight = current - 1.f;
            return true;
        default:
            return false;
    }
}

void MainWindow::createFileMenu (juce::PopupMenu& menu) {}

void MainWindow::createEditMenu (juce::PopupMenu& menu) {}

void MainWindow::createViewMenu (juce::PopupMenu& menu)
{
    menu.addCommandItem (cmdManager, AppCommands::incTextHeight);
    menu.addCommandItem (cmdManager, AppCommands::decTextHeight);
}

void MainWindow::createOptionsMenu (juce::PopupMenu& menu) {}

void MainWindow::createHelpMenu (PopupMenu& menu) {}