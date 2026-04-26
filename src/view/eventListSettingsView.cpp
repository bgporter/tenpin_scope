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
#include "lookAndFeel.h"

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
, octaveTypeComboBox { "Octave type" }
, valueFormatTypeComboBox { "Value format" }
, precisionDigitsComboBox { "Precision" }
{
    setSize (400, 300);
    setupButton (showParsedDataButton, "Show parsed data", eventViewContext.umpShowParsedData.getId ());
    setupButton (showRawDataButton, "Show raw data", eventViewContext.umpShowRawData.getId ());

    setupCombobox (octaveTypeComboBox, eventViewContext.octaveType.getId (), octaveTypeOptions, OctaveType::Yamaha);
    setupCombobox (valueFormatTypeComboBox, eventViewContext.valueFormatType.getId (), valueFormatTypeOptions, ValueFormatType::Integer);
    setupCombobox (precisionDigitsComboBox, eventViewContext.precision.getId (), precisionDigitsOptions, 2);
}

void EventListSettingsView::paint (juce::Graphics& g)
{
    const auto* lnf    = TenpinLookAndFeel::getFrom (*this);
    const auto palette = lnf->getPalette ();

    g.fillAll (palette.defaultFill.get ());
}

void EventListSettingsView::resized ()
{
    constexpr int margin  = 8;
    constexpr int buttonH = 24;
    constexpr int comboH  = 28;
    constexpr int gap     = 8;

    auto bounds = getLocalBounds ().reduced (margin);

    // Row 1: display mode toggles side by side
    auto buttonRow = bounds.removeFromTop (buttonH);
    bounds.removeFromTop (gap);
    const int halfWidth = (buttonRow.getWidth () - gap) / 2;
    showParsedDataButton.setBounds (buttonRow.removeFromLeft (halfWidth));
    buttonRow.removeFromLeft (gap);
    showRawDataButton.setBounds (buttonRow);

    // Combobox rows:
    octaveTypeComboBox.setBounds (bounds.removeFromTop (comboH));
    bounds.removeFromTop (gap);

    valueFormatTypeComboBox.setBounds (bounds.removeFromTop (comboH));
    bounds.removeFromTop (gap);
    precisionDigitsComboBox.setBounds (bounds.removeFromTop (comboH));
    bounds.removeFromTop (gap);
}

void EventListSettingsView::setupButton (juce::Button& button, const juce::StringRef txt, const juce::Identifier& valId)
{
    addAndMakeVisible (button);
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
        valId, [valId, setButtonStateFromContext] (const juce::Identifier& /*id*/) { setButtonStateFromContext (); });
}

