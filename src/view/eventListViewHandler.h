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
#include "eventView.h"
#include "handler/umpHandler.h"
#include "model/appContext.h"

struct Sysex7Event;
struct SystemCommonEvent;
struct UtilityEvent16T;
struct DeltaTicksSinceLastEvent;
struct Midi1NoteEvent;
struct Midi2NoteEvent;
struct Midi2PerNoteEvent;
struct Midi2ControllerEvent;
struct Midi2RelativeControllerEvent;

class EventListViewHandler : public UmpHandler
{
public:
    EventListViewHandler (AppContext& theAppContext);
    ~EventListViewHandler () override;

    UmpHandler::Result handle (const UmpEvent& event, int index, EventView* view, int width);

private:
    UmpHandler::Result preDispatch (const UmpEvent& event) override;
    UmpHandler::Result postDispatch (const UmpEvent& event, UmpHandler::Result pendingResult) override;
    /**
     * @name SysEx 7 Message Handlers
     */
    ///@{
    UmpHandler::Result onSysex7CompleteEvent (const UmpEvent& event) override;
    UmpHandler::Result onSysex7StartEvent    (const UmpEvent& event) override;
    UmpHandler::Result onSysex7ContinueEvent (const UmpEvent& event) override;
    UmpHandler::Result onSysex7EndEvent      (const UmpEvent& event) override;
    UmpHandler::Result onSysex7Event         (const UmpEvent& event) override;
    ///@}

    /**
     * @name System Common Message Handlers
     */
    ///@{
    UmpHandler::Result onSystemCommonEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidiTimeCodeEvent (const UmpEvent& event) override;
    UmpHandler::Result onSongPositionPointerEvent (const UmpEvent& event) override;
    UmpHandler::Result onSongSelectEvent (const UmpEvent& event) override;
    UmpHandler::Result onTuneRequestEvent (const UmpEvent& event) override;
    UmpHandler::Result onTimingClockEvent (const UmpEvent& event) override;
    UmpHandler::Result onStartEvent (const UmpEvent& event) override;
    UmpHandler::Result onContinueEvent (const UmpEvent& event) override;
    UmpHandler::Result onStopEvent (const UmpEvent& event) override;
    UmpHandler::Result onActiveSensingEvent (const UmpEvent& event) override;
    UmpHandler::Result onSystemResetEvent (const UmpEvent& event) override;
    ///@}

    /**
     * @name Utility Message Handlers
     */
    ///@{
    UmpHandler::Result onNoOpEvent (const UmpEvent& event) override;
    UmpHandler::Result onJrClockEvent (const UmpEvent& event) override;
    UmpHandler::Result onJrTimestampEvent (const UmpEvent& event) override;
    UmpHandler::Result onDeltaTicksPerQuarterEvent (const UmpEvent& event) override;
    UmpHandler::Result onDeltaTicksSinceLastEvent (const UmpEvent& event) override;
    ///@}

    /**
     * @name MIDI 1.0 Channel Voice Message Handlers
     */
    ///@{
    UmpHandler::Result onMidi1NoteOffEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi1NoteOnEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi1NoteEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi1PolyPressureEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi1ControlChangeEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi1ProgramChangeEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi1ChannelPressureEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi1PitchBendEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi1ChannelVoiceEvent (const UmpEvent& event) override;
    ///@}

    /**
     * @name MIDI 2.0 Channel Voice Message Handlers
     */
    ///@{
    UmpHandler::Result onMidi2NoteOffEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi2NoteOnEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi2NoteEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi2RegisteredPerNoteControllerEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi2AssignablePerNoteControllerEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi2PerNoteEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi2PerNotePitchBendEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi2ControlChangeEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi2ProgramChangeEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi2PerNoteManagementEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi2PolyPressureEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi2ChannelPressureEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi2PitchBendEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi2RegisteredControllerEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi2AssignableControllerEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi2ControllerEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi2RelativeRegisteredControllerEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi2RelativeAssignableControllerEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi2RelativeControllerEvent (const UmpEvent& event) override;
    UmpHandler::Result onMidi2ChannelVoiceEvent (const UmpEvent& event) override;
    UmpHandler::Result onUmpEvent (const UmpEvent& event) override;
    ///@}

    UmpHandler::Result addSysex7DataValues (const Sysex7Event& e);
    UmpHandler::Result addSystemCommonNoDataValues (const SystemCommonEvent& e);
    UmpHandler::Result addUtilityTicksValues (const UtilityEvent16T& e);
    UmpHandler::Result addUtilityTicksValues (const DeltaTicksSinceLastEvent& e);
    UmpHandler::Result addMidi1NoteValues (const Midi1NoteEvent& e);
    UmpHandler::Result addMidi2NoteValues (const Midi2NoteEvent& e);
    UmpHandler::Result addMidi2PerNoteValues (const Midi2PerNoteEvent& e);
    UmpHandler::Result addMidi2ControllerValues (const Midi2ControllerEvent& e);
    UmpHandler::Result addMidi2RelativeControllerValues (const Midi2RelativeControllerEvent& e);

    AppContext appContext;
    PersistentContext pc;
    EventView* eventView { nullptr };
    int currentIndex { -1 };
    int currentWidth { 0 };
};
