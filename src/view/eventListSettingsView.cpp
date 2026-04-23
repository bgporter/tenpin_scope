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

#include "eventListSettingsView.h"

EventListSettingsView::EventListSettingsView (AppContext& theAppContext)
: appContext { theAppContext }
{
    setSize (400, 300);
}

void EventListSettingsView::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel ().findColour (juce::ResizableWindow::backgroundColourId));
}

void EventListSettingsView::resized () {}

void EventListSettingsView::setupButton (juce::Button& button, const juce::StringRef txt, const juce::Identifier& valId)
{
    button.setButtonText (txt);

    auto setButtonStateFromContext = [this, &button, valId] ()
    {
        bool newValue = persistentContext.eventViewContext.getattr<bool> (valId);
        button.setToggleState (newValue, juce::dontSendNotification);
    };

    setButtonStateFromContext ();

    // when the button is clicked, update the value in the context
    button.onClick = [this, &button, valId] ()
    { persistentContext.eventViewContext.setattr<bool> (valId, button.getToggleState ()); };

    // when the value in the context changes, update the button's toggle state
    persistentContext.eventViewContext.onPropertyChange (
        valId, [this, &button, valId, setButtonStateFromContext] (const juce::Identifier& /*id*/)
        { setButtonStateFromContext (); });
}
