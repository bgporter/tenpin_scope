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

#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

#include "model/appContext.h"

class MainComponent;

//==============================================================================
/*
    This class implements the desktop window that contains an instance of
    our MainComponent class.
*/
class MainWindow final : public juce::DocumentWindow,
                         public juce::ApplicationCommandTarget,
                         public juce::MenuBarModel
{
public:
    explicit MainWindow (juce::String name, AppContext& theAppContext);
    ~MainWindow () override;

    void closeButtonPressed () override;

    bool okayToClose ();

    /**
     * @name MenuBarModel overrides.
     */
    ///@{
    juce::StringArray getMenuBarNames () override;
    juce::PopupMenu getMenuForIndex (int topLevelMenuIndex, const juce::String& menuName) override;
    void menuItemSelected (int menuItemID, int topLevelMenuIndex) override;
    ///@}

    /**
     * @name ApplicationCommandTarget overrides.
     */
    ///@{
    ApplicationCommandTarget* getNextCommandTarget () override;
    void getAllCommands (Array<CommandID>& commands) override;
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override;
    bool perform (const InvocationInfo& info) override;
    ///@}

    /* Note: Be careful if you override any DocumentWindow methods - the base
       class uses a lot of them, so by overriding you might break its functionality.
       It's best to do all your work in your content component instead, but if
       you really have to override any DocumentWindow methods, make sure your
       subclass also calls the superclass's method.
    */
    void moved () override;

    void resized () override;

private:
    void createFileMenu (PopupMenu& menu);
    void createEditMenu (PopupMenu& menu);
    void createViewMenu (PopupMenu& menu);
    void createOptionsMenu (PopupMenu& menu);
    void createHelpMenu (PopupMenu& menu);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    AppContext appContext;
};
