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

namespace
{
juce::StringArray octaveTypeOptions { "Yamaha (middle C = C3)", "Roland (middle C = C4)" };
juce::StringArray valueFormatTypeOptions { "Integer", "Hex", "Float (0..1 or -1..1)",
                                           "Midi (0..127 with fractional part)", "Percent (0..100)" };
juce::StringArray precisionDigitsOptions { "1", "2", "3", "4" };

} // namespace

EventListSettingsView::EventListSettingsView (AppContext& theAppContext)
: appContext { theAppContext }
, persistentContext { appContext }
, eventViewContext { persistentContext }
{
    setSize (400, 300);
    addAndMakeVisible (showParsedDataButton);
    addAndMakeVisible (showRawDataButton);

    setupButton (showParsedDataButton, "Show parsed data", eventViewContext.umpShowParsedData.getId ());
    setupButton (showRawDataButton, "Show raw data", eventViewContext.umpShowRawData.getId ());
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
        bool newValue = persistentContext.eventViewContext.getattr<bool> (valId, false);
        button.setToggleState (newValue, juce::dontSendNotification);
    };

    setButtonStateFromContext ();

    // when the button is clicked, update the value in the context
    button.onClick = [this, &button, valId] ()
    { persistentContext.eventViewContext.setattr<bool> (valId, button.getToggleState ()); };

    // when the value in the context changes, update the button's toggle state
    persistentContext.eventViewContext.onPropertyChange (
                                                         valId, [  valId, setButtonStateFromContext] (const juce::Identifier& /*id*/)
        { setButtonStateFromContext (); });
}

void EventListSettingsView::setupCombobox (juce::ComboBox& comboBox, const juce::Identifier& valId,
                                           juce::StringArray& options)
{
    comboBox.addItemList (options, 1);
    comboBox.setSelectedItemIndex (0);

    auto setComboboxStateFromContext = [this, &comboBox, valId] ()
    {
        int newValue = persistentContext.eventViewContext.getattr<int> (valId, 1);
        comboBox.setSelectedItemIndex (newValue);
    };

    setComboboxStateFromContext ();

    // when the combobox selection changes, update the value in the context
    comboBox.onChange = [this, &comboBox, valId] ()
    { persistentContext.eventViewContext.setattr<int> (valId, comboBox.getSelectedId ()); };

    // when the value in the context changes, update the combobox selection
    persistentContext.eventViewContext.onPropertyChange (
        valId, [this, &comboBox, valId, setComboboxStateFromContext] (const juce::Identifier& /*id*/)
        { setComboboxStateFromContext (); });
}
