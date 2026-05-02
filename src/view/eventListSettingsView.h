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

#include <JuceHeader.h>

#include "eventNameUtils.h"
#include "labeledComboBox.h"
#include "model/appContext.h"
#include "model/persistentContext.h"

class EventListSettingsView : public juce::Component
{
public:
    EventListSettingsView (AppContext& theAppContext);
    void paint (juce::Graphics& g) override;
    void resized () override;

private:
    /**
     * @brief Connect the button to the cello::Value in the context.
     *
     * @param button
     * @param txt
     * @param valId
     */
    void setupButton (juce::Button& button, const juce::StringRef txt, const juce::Identifier& valId);

    /**
     * @brief Initialize and populate a combo box, connecting it to a
     * typed cello::Value in the context. T must be castable to/from int
     * and match the combo box item IDs (1-based).
     */
    template <typename T>
    void setupCombobox (LabeledComboBox& labeledComboBox, const juce::Identifier& valId, juce::StringArray& options,
                        T defaultValue)
    {
        addAndMakeVisible (labeledComboBox);
        auto& comboBox = labeledComboBox.comboBox;
        comboBox.addItemList (options, 1);

        auto setFromContext = [this, &comboBox, valId, defaultValue] ()
        {
            T value = persistentContext.eventViewContext.getattr<T> (valId, defaultValue);
            comboBox.setSelectedId (static_cast<int> (value), juce::dontSendNotification);
        };

        setFromContext ();

        comboBox.onChange = [this, &comboBox, valId] ()
        { persistentContext.eventViewContext.setattr<T> (valId, static_cast<T> (comboBox.getSelectedId ())); };

        persistentContext.eventViewContext.onPropertyChange (valId, [setFromContext] (const juce::Identifier&)
                                                             { setFromContext (); });
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EventListSettingsView)
    AppContext appContext;
    PersistentContext persistentContext;
    EventViewContext eventViewContext;

    // buttons & controls:
    juce::ToggleButton showParsedDataButton;
    juce::ToggleButton showRawDataButton;

    LabeledComboBox octaveTypeComboBox { "Octave type" };
    LabeledComboBox valueFormatTypeComboBox { "Value format" };
    LabeledComboBox precisionDigitsComboBox { "Precision" };

    // message type filter section label
    juce::Label messageFiltersLabel;

    // top-level message type toggles
    juce::ToggleButton showUtilityButton;
    juce::ToggleButton showSystemCommonRealtimeButton;
    juce::ToggleButton showChannelVoiceButton;
    juce::ToggleButton showSysex7Button;
    juce::ToggleButton showSysex8Button;
    juce::ToggleButton showFlexDataButton;
    juce::ToggleButton showStreamDataButton;
    juce::ToggleButton showUndefinedButton;

    // channel voice sub-filters (indented, disabled when parent is off)
    juce::ToggleButton showNoteOnOffButton;
    juce::ToggleButton showControlChangeButton;
    juce::ToggleButton showProgramChangeButton;
    juce::ToggleButton showChannelPressureButton;
    juce::ToggleButton showPitchBendButton;
    juce::ToggleButton showPerNoteEventsButton;

    // data8 sub-filters (indented, disabled when parent is off)
    juce::ToggleButton showSysex8PacketsButton;
    juce::ToggleButton showMixedDataButton;
};
