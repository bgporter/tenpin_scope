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
#include "view/dispatchContext.h"
#include "model/appContext.h"

struct Sysex7Event;
struct Sysex8Event;
struct MixedDataSetHeaderEvent;
struct MixedDataSetPayloadEvent;
struct FlexDataTextEvent;
struct SetChordEvent;
struct SetKeySignatureEvent;
struct SetMetronomeEvent;
struct SetTempoEvent;
struct SetTimeSignatureEvent;
struct SystemCommonEvent;
struct UtilityEvent16T;
struct DeltaTicksSinceLastEvent;
struct Midi1NoteEvent;
struct Midi2NoteEvent;
struct Midi2PerNoteEvent;
struct Midi2ControllerEvent;
struct Midi2RelativeControllerEvent;
struct StreamTextEvent;

class EventListViewHandler : public UmpHandler
{
public:
    EventListViewHandler (AppContext& theAppContext);
    ~EventListViewHandler () override;

    Handler::Result handle (const UmpEvent& event, void* ctx) override;

private:
    Handler::Result preDispatch (const UmpEvent& event) override;
    Handler::Result postDispatch (const UmpEvent& event, Handler::Result pendingResult) override;
    /**
     * @name SysEx 7 Message Handlers
     */
    ///@{
    Handler::Result onSysex7CompleteEvent (const UmpEvent& event) override;
    Handler::Result onSysex7StartEvent    (const UmpEvent& event) override;
    Handler::Result onSysex7ContinueEvent (const UmpEvent& event) override;
    Handler::Result onSysex7EndEvent      (const UmpEvent& event) override;
    Handler::Result onSysex7Event         (const UmpEvent& event) override;
    ///@}

    /**
     * @name SysEx 8 Message Handlers
     */
    ///@{
    Handler::Result onSysex8CompleteEvent (const UmpEvent& event) override;
    Handler::Result onSysex8StartEvent    (const UmpEvent& event) override;
    Handler::Result onSysex8ContinueEvent (const UmpEvent& event) override;
    Handler::Result onSysex8EndEvent      (const UmpEvent& event) override;
    Handler::Result onSysex8Event         (const UmpEvent& event) override;
    Handler::Result onMixedDataSetHeaderEvent  (const UmpEvent& event) override;
    Handler::Result onMixedDataSetPayloadEvent (const UmpEvent& event) override;
    Handler::Result onSetTempoEvent            (const UmpEvent& event) override;
    Handler::Result onSetTimeSignatureEvent    (const UmpEvent& event) override;
    Handler::Result onSetMetronomeEvent        (const UmpEvent& event) override;
    Handler::Result onSetKeySignatureEvent     (const UmpEvent& event) override;
    Handler::Result onSetChordEvent            (const UmpEvent& event) override;
    Handler::Result onMetadataTextEvent        (const UmpEvent& event) override;
    Handler::Result onPerformanceTextEvent     (const UmpEvent& event) override;
    ///@}

    /**
     * @name System Common Message Handlers
     */
    ///@{
    Handler::Result onSystemCommonEvent (const UmpEvent& event) override;
    Handler::Result onMidiTimeCodeEvent (const UmpEvent& event) override;
    Handler::Result onSongPositionPointerEvent (const UmpEvent& event) override;
    Handler::Result onSongSelectEvent (const UmpEvent& event) override;
    Handler::Result onTuneRequestEvent (const UmpEvent& event) override;
    Handler::Result onTimingClockEvent (const UmpEvent& event) override;
    Handler::Result onStartEvent (const UmpEvent& event) override;
    Handler::Result onContinueEvent (const UmpEvent& event) override;
    Handler::Result onStopEvent (const UmpEvent& event) override;
    Handler::Result onActiveSensingEvent (const UmpEvent& event) override;
    Handler::Result onSystemResetEvent (const UmpEvent& event) override;
    ///@}

    /**
     * @name Utility Message Handlers
     */
    ///@{
    Handler::Result onNoOpEvent (const UmpEvent& event) override;
    Handler::Result onJrClockEvent (const UmpEvent& event) override;
    Handler::Result onJrTimestampEvent (const UmpEvent& event) override;
    Handler::Result onDeltaTicksPerQuarterEvent (const UmpEvent& event) override;
    Handler::Result onDeltaTicksSinceLastEvent (const UmpEvent& event) override;
    ///@}

    /**
     * @name MIDI 1.0 Channel Voice Message Handlers
     */
    ///@{
    Handler::Result onMidi1NoteOffEvent (const UmpEvent& event) override;
    Handler::Result onMidi1NoteOnEvent (const UmpEvent& event) override;
    Handler::Result onMidi1NoteEvent (const UmpEvent& event) override;
    Handler::Result onMidi1PolyPressureEvent (const UmpEvent& event) override;
    Handler::Result onMidi1ControlChangeEvent (const UmpEvent& event) override;
    Handler::Result onMidi1ProgramChangeEvent (const UmpEvent& event) override;
    Handler::Result onMidi1ChannelPressureEvent (const UmpEvent& event) override;
    Handler::Result onMidi1PitchBendEvent (const UmpEvent& event) override;
    Handler::Result onMidi1ChannelVoiceEvent (const UmpEvent& event) override;
    ///@}

    /**
     * @name MIDI 2.0 Channel Voice Message Handlers
     */
    ///@{
    Handler::Result onMidi2NoteOffEvent (const UmpEvent& event) override;
    Handler::Result onMidi2NoteOnEvent (const UmpEvent& event) override;
    Handler::Result onMidi2NoteEvent (const UmpEvent& event) override;
    Handler::Result onMidi2RegisteredPerNoteControllerEvent (const UmpEvent& event) override;
    Handler::Result onMidi2AssignablePerNoteControllerEvent (const UmpEvent& event) override;
    Handler::Result onMidi2PerNoteEvent (const UmpEvent& event) override;
    Handler::Result onMidi2PerNotePitchBendEvent (const UmpEvent& event) override;
    Handler::Result onMidi2ControlChangeEvent (const UmpEvent& event) override;
    Handler::Result onMidi2ProgramChangeEvent (const UmpEvent& event) override;
    Handler::Result onMidi2PerNoteManagementEvent (const UmpEvent& event) override;
    Handler::Result onMidi2PolyPressureEvent (const UmpEvent& event) override;
    Handler::Result onMidi2ChannelPressureEvent (const UmpEvent& event) override;
    Handler::Result onMidi2PitchBendEvent (const UmpEvent& event) override;
    Handler::Result onMidi2RegisteredControllerEvent (const UmpEvent& event) override;
    Handler::Result onMidi2AssignableControllerEvent (const UmpEvent& event) override;
    Handler::Result onMidi2ControllerEvent (const UmpEvent& event) override;
    Handler::Result onMidi2RelativeRegisteredControllerEvent (const UmpEvent& event) override;
    Handler::Result onMidi2RelativeAssignableControllerEvent (const UmpEvent& event) override;
    Handler::Result onMidi2RelativeControllerEvent (const UmpEvent& event) override;
    Handler::Result onMidi2ChannelVoiceEvent (const UmpEvent& event) override;
    Handler::Result onUmpEvent (const UmpEvent& event) override;
    ///@}

    /**
     * @name UMP Stream Message Handlers
     */
    ///@{
    Handler::Result onEndpointDiscoveryEvent             (const UmpEvent& event) override;
    Handler::Result onEndpointInfoNotificationEvent      (const UmpEvent& event) override;
    Handler::Result onDeviceIdentityNotificationEvent    (const UmpEvent& event) override;
    Handler::Result onEndpointNameNotificationEvent      (const UmpEvent& event) override;
    Handler::Result onProductInstanceIdEvent             (const UmpEvent& event) override;
    Handler::Result onStreamConfigRequestEvent           (const UmpEvent& event) override;
    Handler::Result onStreamConfigNotificationEvent      (const UmpEvent& event) override;
    Handler::Result onFunctionBlockDiscoveryEvent        (const UmpEvent& event) override;
    Handler::Result onFunctionBlockInfoNotificationEvent (const UmpEvent& event) override;
    Handler::Result onFunctionBlockNameNotificationEvent (const UmpEvent& event) override;
    Handler::Result onStartOfClipEvent                   (const UmpEvent& event) override;
    Handler::Result onEndOfClipEvent                     (const UmpEvent& event) override;
    ///@}

    Handler::Result handleTextEvent (const UmpEvent& event);
    Handler::Result addSysex7DataValues (const Sysex7Event& e);
    Handler::Result addSysex8DataValues (const Sysex8Event& e);
    Handler::Result addSystemCommonNoDataValues (const SystemCommonEvent& e);
    Handler::Result addUtilityTicksValues (const UtilityEvent16T& e);
    Handler::Result addUtilityTicksValues (const DeltaTicksSinceLastEvent& e);
    Handler::Result addMidi1NoteValues (const Midi1NoteEvent& e);
    Handler::Result addMidi2NoteValues (const Midi2NoteEvent& e);
    Handler::Result addMidi2PerNoteValues (const Midi2PerNoteEvent& e);
    Handler::Result addMidi2ControllerValues (const Midi2ControllerEvent& e);
    Handler::Result addMidi2RelativeControllerValues (const Midi2RelativeControllerEvent& e);
    Handler::Result addStreamTextValues (const StreamTextEvent& e);
    Handler::Result addStreamConfigValues (const UmpEvent& event, int theStatus);

    AppContext appContext;
    PersistentContext pc;
    EventView* eventView { nullptr };
    int currentIndex { -1 };
    int currentWidth { 0 };
};
