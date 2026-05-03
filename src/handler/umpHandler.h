/*
 MIT License

 Copyright (c) 2026 Brett g Porter

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to so, subject to the following conditions:

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

#include "model/ump/umpEvent.h"

class UmpHandler
{
public:
    UmpHandler ();
    virtual ~UmpHandler ();

    enum class Result
    {
        ok,
        notHandled,
        ignore
    };

    Result handle (const UmpEvent& event);

protected:
    Result defaultResult { Result::notHandled };

private:
    Result handleUtilityEvent (const UmpEvent& event);
    Result handleSystemCommonEvent (const UmpEvent& event);
    Result handleSysex7Event (const UmpEvent& event);
    Result handleSysex8Event (const UmpEvent& event);
    Result handleMidi1ChannelVoiceEvent (const UmpEvent& event);
    Result handleMidi2ChannelVoiceEvent (const UmpEvent& event);

    /**
     * @brief A place to do any per-handler preparation for handing an event. This is called before the event is
     * dispatched to the onUmpEvent handler.
     *
     * @param event
     * @return Result
     */
    virtual Result preDispatch (const UmpEvent& event) { return Result::ok; }

    /**
     * @brief A place to do any per-handler post-processing for handing an event. This is called after the event is
     *
     * @param pendingResult Result of having handled the event.
     * @return Result
     */
    virtual Result postDispatch (const UmpEvent& event, Result pendingResult) { return pendingResult; }

    virtual Result onUmpEvent (const UmpEvent&) { return defaultResult; }

    /**
     * @name SysEx 7 Message Handlers
     */
    ///@{
    virtual Result onSysex7Event (const UmpEvent&) { return defaultResult; }
    virtual Result onSysex7CompleteEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onSysex7StartEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onSysex7ContinueEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onSysex7EndEvent (const UmpEvent&) { return defaultResult; }
    ///@}

    /**
     * @name SysEx 8 Message Handlers
     */
    ///@{
    virtual Result onSysex8Event (const UmpEvent&) { return defaultResult; }
    virtual Result onSysex8CompleteEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onSysex8StartEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onSysex8ContinueEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onSysex8EndEvent (const UmpEvent&) { return defaultResult; }
    ///@}

    /**
     * @name System Common Message Handlers
     */
    ///@{
    virtual Result onSystemCommonEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidiTimeCodeEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onSongPositionPointerEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onSongSelectEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onTuneRequestEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onTimingClockEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onStartEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onContinueEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onStopEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onActiveSensingEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onSystemResetEvent (const UmpEvent&) { return defaultResult; }
    ///@}

    /**
     * @name Utility Message Handlers
     */
    ///@{
    virtual Result onNoOpEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onJrClockEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onJrTimestampEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onDeltaTicksPerQuarterEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onDeltaTicksSinceLastEvent (const UmpEvent&) { return defaultResult; }
    ///@}

    /**
     * @name MIDI 1.0 Channel Voice Message Handlers
     */
    ///@{
    virtual Result onMidi1ChannelVoiceEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi1NoteEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi1NoteOffEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi1NoteOnEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi1PolyPressureEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi1ControlChangeEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi1ProgramChangeEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi1ChannelPressureEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi1PitchBendEvent (const UmpEvent&) { return defaultResult; }
    ///@}

    /**
     * @name MIDI 2.0 Channel Voice Message Handlers
     */
    ///@{
    virtual Result onMidi2ChannelVoiceEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi2NoteOffEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi2NoteOnEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi2NoteEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi2PerNoteEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi2PerNotePitchBendEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi2ControlChangeEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi2ProgramChangeEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi2PerNoteManagementEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi2PolyPressureEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi2ChannelPressureEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi2PitchBendEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi2RegisteredPerNoteControllerEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi2AssignablePerNoteControllerEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi2ControllerEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi2RegisteredControllerEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi2AssignableControllerEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi2RelativeControllerEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi2RelativeRegisteredControllerEvent (const UmpEvent&) { return defaultResult; }
    virtual Result onMidi2RelativeAssignableControllerEvent (const UmpEvent&) { return defaultResult; }
    ///@}
};
