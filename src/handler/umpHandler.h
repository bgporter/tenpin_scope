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

#include "handler/handlerResult.h"
#include "model/ump/umpEvent.h"

class UmpHandler
{
public:
    UmpHandler ();
    virtual ~UmpHandler ();

    Handler::Result handle (const UmpEvent& event);
    virtual Handler::Result handle (const UmpEvent& event, void* ctx);

protected:
    Handler::Result defaultResult { Handler::Result::notHandled };

private:
    Handler::Result handleUtilityEvent (const UmpEvent& event);
    Handler::Result handleSystemCommonEvent (const UmpEvent& event);
    Handler::Result handleSysex7Event (const UmpEvent& event);
    Handler::Result handleSysex8Event (const UmpEvent& event);
    Handler::Result handleFlexDataEvent (const UmpEvent& event);
    Handler::Result handleSetupAndPerformanceEvent (const UmpEvent& event);
    Handler::Result handleStreamEvent (const UmpEvent& event);
    Handler::Result handleMidi1ChannelVoiceEvent (const UmpEvent& event);
    Handler::Result handleMidi2ChannelVoiceEvent (const UmpEvent& event);

    /**
     * @brief A place to do any per-handler preparation for handing an event. This is called before the event is
     * dispatched to the onUmpEvent handler.
     *
     * @param event
     * @return Result
     */
    virtual Handler::Result preDispatch (const UmpEvent& event) { return Handler::Result::ok; }

    /**
     * @brief A place to do any per-handler post-processing for handing an event. This is called after the event is
     *
     * @param pendingResult Result of having handled the event.
     * @return Result
     */
    virtual Handler::Result postDispatch (const UmpEvent& event, Handler::Result pendingResult) { return pendingResult; }

    virtual Handler::Result onUmpEvent (const UmpEvent&) { return defaultResult; }

    /**
     * @name SysEx 7 Message Handlers
     */
    ///@{
    virtual Handler::Result onSysex7Event (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onSysex7CompleteEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onSysex7StartEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onSysex7ContinueEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onSysex7EndEvent (const UmpEvent&) { return defaultResult; }
    ///@}

    /**
     * @name SysEx 8 Message Handlers
     */
    ///@{
    virtual Handler::Result onSysex8Event (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onSysex8CompleteEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onSysex8StartEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onSysex8ContinueEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onSysex8EndEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMixedDataSetHeaderEvent  (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMixedDataSetPayloadEvent (const UmpEvent&) { return defaultResult; }
    ///@}

    /**
     * @name UMP Stream Message Handlers
     */
    ///@{
    virtual Handler::Result onStreamEvent                          (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onEndpointDiscoveryEvent               (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onEndpointInfoNotificationEvent        (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onDeviceIdentityNotificationEvent      (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onEndpointNameNotificationEvent        (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onProductInstanceIdEvent               (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onStreamConfigRequestEvent             (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onStreamConfigNotificationEvent        (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onFunctionBlockDiscoveryEvent          (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onFunctionBlockInfoNotificationEvent   (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onFunctionBlockNameNotificationEvent   (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onStartOfClipEvent                     (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onEndOfClipEvent                       (const UmpEvent&) { return defaultResult; }
    ///@}

    /**
     * @name Flex Data Message Handlers
     */
    ///@{
    virtual Handler::Result onFlexDataEvent             (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onSetTempoEvent             (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onSetTimeSignatureEvent     (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onSetMetronomeEvent         (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onSetKeySignatureEvent      (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onSetChordEvent             (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMetadataTextEvent         (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onPerformanceTextEvent      (const UmpEvent&) { return defaultResult; }
    ///@}

    /**
     * @name System Common Message Handlers
     */
    ///@{
    virtual Handler::Result onSystemCommonEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidiTimeCodeEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onSongPositionPointerEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onSongSelectEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onTuneRequestEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onTimingClockEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onStartEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onContinueEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onStopEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onActiveSensingEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onSystemResetEvent (const UmpEvent&) { return defaultResult; }
    ///@}

    /**
     * @name Utility Message Handlers
     */
    ///@{
    virtual Handler::Result onNoOpEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onJrClockEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onJrTimestampEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onDeltaTicksPerQuarterEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onDeltaTicksSinceLastEvent (const UmpEvent&) { return defaultResult; }
    ///@}

    /**
     * @name MIDI 1.0 Channel Voice Message Handlers
     */
    ///@{
    virtual Handler::Result onMidi1ChannelVoiceEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi1NoteEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi1NoteOffEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi1NoteOnEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi1PolyPressureEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi1ControlChangeEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi1ProgramChangeEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi1ChannelPressureEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi1PitchBendEvent (const UmpEvent&) { return defaultResult; }
    ///@}

    /**
     * @name MIDI 2.0 Channel Voice Message Handlers
     */
    ///@{
    virtual Handler::Result onMidi2ChannelVoiceEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi2NoteOffEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi2NoteOnEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi2NoteEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi2PerNoteEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi2PerNotePitchBendEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi2ControlChangeEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi2ProgramChangeEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi2PerNoteManagementEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi2PolyPressureEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi2ChannelPressureEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi2PitchBendEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi2RegisteredPerNoteControllerEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi2AssignablePerNoteControllerEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi2ControllerEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi2RegisteredControllerEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi2AssignableControllerEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi2RelativeControllerEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi2RelativeRegisteredControllerEvent (const UmpEvent&) { return defaultResult; }
    virtual Handler::Result onMidi2RelativeAssignableControllerEvent (const UmpEvent&) { return defaultResult; }
    ///@}
};
