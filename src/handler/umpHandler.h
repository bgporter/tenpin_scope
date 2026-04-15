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
    virtual Result postDispatch (Result pendingResult) { return pendingResult; }

    virtual Result onUmpEvent (const UmpEvent&) { return defaultResult; }

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
