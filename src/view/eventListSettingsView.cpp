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
    setupButton (showParsedDataButton, "Show parsed event data", eventViewContext.umpShowParsedData.getId ());
    setupButton (showRawDataButton, "Show raw UMP data", eventViewContext.umpShowRawData.getId ());

    setupCombobox (octaveTypeComboBox, eventViewContext.octaveType.getId (), octaveTypeOptions, OctaveType::Yamaha);
    setupCombobox (valueFormatTypeComboBox, eventViewContext.valueFormatType.getId (), valueFormatTypeOptions,
                   ValueFormatType::Integer);
    setupCombobox (precisionDigitsComboBox, eventViewContext.precision.getId (), precisionDigitsOptions, 2);

    addAndMakeVisible (messageFiltersLabel);
    messageFiltersLabel.setText ("Message Filters", juce::dontSendNotification);
    messageFiltersLabel.setFont (juce::Font (13.f, juce::Font::bold));

    setupButton (showUtilityButton, "Utility", eventViewContext.showUtility.getId ());
    setupButton (showSystemCommonRealtimeButton, "System Common/Realtime",
                 eventViewContext.showSystemCommonRealtime.getId ());
    setupButton (showChannelVoiceButton, "Channel Voice (MIDI 1/2)", eventViewContext.showChannelVoice.getId ());
    setupButton (showNoteOnOffButton, "Note On/Off", eventViewContext.showNoteOnOff.getId ());
    setupButton (showControlChangeButton, "CC", eventViewContext.showControlChange.getId ());
    setupButton (showProgramChangeButton, "Program Change", eventViewContext.showProgramChange.getId ());
    setupButton (showChannelPressureButton, "Channel Pressure", eventViewContext.showChannelPressure.getId ());
    setupButton (showPitchBendButton, "Pitch Bend", eventViewContext.showPitchBend.getId ());
    setupButton (showPerNoteEventsButton, "Per-Note", eventViewContext.showPerNoteEvents.getId ());
    setupButton (showSysex7Button, "Data 7 (SysEx 7)", eventViewContext.showSysex7.getId ());
    setupButton (showSysex8Button, "Data 8", eventViewContext.showSysex8.getId ());
    setupButton (showSysex8PacketsButton, "SysEx 8", eventViewContext.showSysex8Packets.getId ());
    setupButton (showMixedDataButton, "Mixed Data", eventViewContext.showMixedData.getId ());
    setupButton (showFlexDataButton, "Flex Data", eventViewContext.showFlexData.getId ());
    setupButton (showStreamDataButton, "Stream Data", eventViewContext.showStreamData.getId ());
    setupButton (showUndefinedButton, "Undefined", eventViewContext.showUndefined.getId ());

    // When the Channel Voice parent is toggled, enable/disable its children to
    // make it clear they have no effect while the parent is off.
    auto syncChannelVoiceChildren = [this] ()
    {
        const bool on = eventViewContext.showChannelVoice;
        for (auto* btn : { &showNoteOnOffButton, &showControlChangeButton, &showProgramChangeButton,
                           &showChannelPressureButton, &showPitchBendButton, &showPerNoteEventsButton })
            btn->setEnabled (on);
    };
    eventViewContext.onPropertyChange (eventViewContext.showChannelVoice.getId (),
                                       [syncChannelVoiceChildren] (const juce::Identifier&)
                                       { syncChannelVoiceChildren (); });
    syncChannelVoiceChildren ();

    // Same for the Data 8 parent and its sub-filters.
    auto syncSysex8Children = [this] ()
    {
        const bool on = eventViewContext.showSysex8;
        showSysex8PacketsButton.setEnabled (on);
        showMixedDataButton.setEnabled (on);
    };
    eventViewContext.onPropertyChange (eventViewContext.showSysex8.getId (),
                                       [syncSysex8Children] (const juce::Identifier&) { syncSysex8Children (); });
    syncSysex8Children ();

    // Compute height: 2*margin + display-format section + sectionGap + label + gap + 16 filter rows.
    // Each filter row is buttonH + gap; last row has no trailing gap.
    constexpr int margin     = 6;
    constexpr int buttonH    = 20;
    constexpr int comboH     = 24;
    constexpr int gap        = 4;
    constexpr int sectionGap = 8;
    constexpr int labelH     = 16;
    constexpr int innerH     = buttonH + gap                     // parsed/raw row
                               + comboH + gap                    // octave
                               + comboH + gap                    // format
                               + comboH                          // precision
                               + sectionGap + labelH + gap       // section header
                               + 15 * (buttonH + gap) + buttonH; // 16 filter rows
    setSize (400, innerH + 2 * margin);
}

void EventListSettingsView::paint (juce::Graphics& g)
{
    const auto* lnf    = TenpinLookAndFeel::getFrom (*this);
    const auto palette = lnf->getPalette ();

    g.fillAll (palette.defaultFill.get ());
}

void EventListSettingsView::resized ()
{
    constexpr int margin     = 6;
    constexpr int buttonH    = 20;
    constexpr int comboH     = 24;
    constexpr int gap        = 4;
    constexpr int indent     = 16;
    constexpr int sectionGap = 8;
    constexpr int labelH     = 16;

    auto bounds = getLocalBounds ().reduced (margin);

    // Row 1: display mode toggles side by side
    auto buttonRow = bounds.removeFromTop (buttonH);
    bounds.removeFromTop (gap);
    const int halfWidth = (buttonRow.getWidth () - gap) / 2;
    showParsedDataButton.setBounds (buttonRow.removeFromLeft (halfWidth));
    buttonRow.removeFromLeft (gap);
    showRawDataButton.setBounds (buttonRow);

    // Combobox rows
    octaveTypeComboBox.setBounds (bounds.removeFromTop (comboH));
    bounds.removeFromTop (gap);
    valueFormatTypeComboBox.setBounds (bounds.removeFromTop (comboH));
    bounds.removeFromTop (gap);
    precisionDigitsComboBox.setBounds (bounds.removeFromTop (comboH));

    // Section label for message type filters
    bounds.removeFromTop (sectionGap);
    messageFiltersLabel.setBounds (bounds.removeFromTop (labelH));
    bounds.removeFromTop (gap);

    // Helpers: full-width and indented filter button placement
    auto placeFull = [&] (juce::Button& btn)
    {
        btn.setBounds (bounds.removeFromTop (buttonH));
        bounds.removeFromTop (gap);
    };
    auto placeIndented = [&] (juce::Button& btn)
    {
        auto row = bounds.removeFromTop (buttonH);
        row.removeFromLeft (indent);
        btn.setBounds (row);
        bounds.removeFromTop (gap);
    };

    placeFull (showUtilityButton);
    placeFull (showSystemCommonRealtimeButton);
    placeFull (showChannelVoiceButton);
    placeIndented (showNoteOnOffButton);
    placeIndented (showControlChangeButton);
    placeIndented (showProgramChangeButton);
    placeIndented (showChannelPressureButton);
    placeIndented (showPitchBendButton);
    placeIndented (showPerNoteEventsButton);
    placeFull (showSysex7Button);
    placeFull (showSysex8Button);
    placeIndented (showSysex8PacketsButton);
    placeIndented (showMixedDataButton);
    placeFull (showFlexDataButton);
    placeFull (showStreamDataButton);
    placeFull (showUndefinedButton); // last row — no trailing gap needed
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
