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

#include "umpHandler.h"

#include "model/ump/channelVoice1.h"
#include "model/ump/channelVoice2.h"
#include "model/ump/systemCommon.h"
#include "model/ump/sysex7.h"
#include "model/ump/utility.h"

UmpHandler::UmpHandler () {}

UmpHandler::~UmpHandler () {}

UmpHandler::Result UmpHandler::handle (const UmpEvent& event)
{
    Result result { preDispatch (event) };
    if (result == Result::ok)
    {
        switch (event.messageType)
        {
            case MessageTypes::utility:
                result = handleUtilityEvent (event);
                break;
            case MessageTypes::systemCommon:
                result = handleSystemCommonEvent (event);
                break;
            case MessageTypes::sysex7:
                result = handleSysex7Event (event);
                break;
            case MessageTypes::midi1ChannelVoice:
                result = handleMidi1ChannelVoiceEvent (event);
                break;
            case MessageTypes::midi2ChannelVoice:
                result = handleMidi2ChannelVoiceEvent (event);
                break;
            default:
                result = onUmpEvent (event);
                break;
        }
    }
    return postDispatch (event, result);
}

UmpHandler::Result UmpHandler::handleSysex7Event (const UmpEvent& event)
{
    Result result { defaultResult };
    Sysex7Event e (event);
    switch (e.status)
    {
        case SysexStatus::complete:  result = onSysex7CompleteEvent (event); break;
        case SysexStatus::start:     result = onSysex7StartEvent (event);    break;
        case SysexStatus::continue_: result = onSysex7ContinueEvent (event); break;
        case SysexStatus::end:       result = onSysex7EndEvent (event);      break;
        default: break;
    }

    if (result == Result::notHandled)
        result = onSysex7Event (event);

    if (result == Result::notHandled)
        result = onUmpEvent (event);

    return result;
}

UmpHandler::Result UmpHandler::handleSystemCommonEvent (const UmpEvent& event)
{
    Result result { defaultResult };
    SystemCommonEvent systemCommonEvent (event);
    switch (systemCommonEvent.status)
    {
        case UmpValues::SystemCommon::midiTimeCode:
            result = onMidiTimeCodeEvent (event);
            break;
        case UmpValues::SystemCommon::songPositionPointer:
            result = onSongPositionPointerEvent (event);
            break;
        case UmpValues::SystemCommon::songSelect:
            result = onSongSelectEvent (event);
            break;
        case UmpValues::SystemCommon::tuneRequest:
            result = onTuneRequestEvent (event);
            break;
        case UmpValues::SystemCommon::timingClock:
            result = onTimingClockEvent (event);
            break;
        case UmpValues::SystemCommon::start:
            result = onStartEvent (event);
            break;
        case UmpValues::SystemCommon::continue_:
            result = onContinueEvent (event);
            break;
        case UmpValues::SystemCommon::stop:
            result = onStopEvent (event);
            break;
        case UmpValues::SystemCommon::activeSensing:
            result = onActiveSensingEvent (event);
            break;
        case UmpValues::SystemCommon::systemReset:
            result = onSystemResetEvent (event);
            break;
        default:
            break;
    }

    if (result == Result::notHandled)
        result = onSystemCommonEvent (event);

    if (result == Result::notHandled)
        result = onUmpEvent (event);

    return result;
}

UmpHandler::Result UmpHandler::handleUtilityEvent (const UmpEvent& event)
{
    Result result { defaultResult };
    UtilityEvent utilityEvent (event);
    switch (utilityEvent.status)
    {
        case UmpValues::Utility::noop:
            result = onNoOpEvent (event);
            break;
        case UmpValues::Utility::jrClock:
            result = onJrClockEvent (event);
            break;
        case UmpValues::Utility::jrTimestamp:
            result = onJrTimestampEvent (event);
            break;
        case UmpValues::Utility::deltaClockstampTPQ:
            result = onDeltaTicksPerQuarterEvent (event);
            break;
        case UmpValues::Utility::deltaClockstampSinceLastEvent:
            result = onDeltaTicksSinceLastEvent (event);
            break;
        default:
            break;
    }

    if (result == Result::notHandled)
        result = onUmpEvent (event);

    return result;
}

UmpHandler::Result UmpHandler::handleMidi1ChannelVoiceEvent (const UmpEvent& event)
{
    Result result { defaultResult };
    Midi1ChannelVoiceEvent midi1ChannelVoiceEvent (event);
    switch (midi1ChannelVoiceEvent.status)
    {
        case UmpValues::ChannelVoice::noteOff:
            result = onMidi1NoteOffEvent (event);
            if (result == Result::notHandled)
                result = onMidi1NoteEvent (event);
            break;

        case UmpValues::ChannelVoice::noteOn:
            result = onMidi1NoteOnEvent (event);
            if (result == Result::notHandled)
                result = onMidi1NoteEvent (event);
            break;

        case UmpValues::ChannelVoice::polyPressure:
            result = onMidi1PolyPressureEvent (event);
            break;

        case UmpValues::ChannelVoice::controlChange:
            result = onMidi1ControlChangeEvent (event);
            break;

        case UmpValues::ChannelVoice::programChange:
            result = onMidi1ProgramChangeEvent (event);
            break;

        case UmpValues::ChannelVoice::channelPressure:
            result = onMidi1ChannelPressureEvent (event);
            break;

        case UmpValues::ChannelVoice::pitchBend:
            result = onMidi1PitchBendEvent (event);
            break;

        default:
            break;
    }

    if (result == Result::notHandled)
        result = onMidi1ChannelVoiceEvent (event);

    if (result == Result::notHandled)
        result = onUmpEvent (event);

    return result;
}

UmpHandler::Result UmpHandler::handleMidi2ChannelVoiceEvent (const UmpEvent& event)
{
    Result result { defaultResult };
    Midi2ChannelVoiceEvent midi2ChannelVoiceEvent (event);
    switch (midi2ChannelVoiceEvent.status)
    {
        case UmpValues::ChannelVoice::registeredPerNoteController:
            result = onMidi2RegisteredPerNoteControllerEvent (event);
            if (result == Result::notHandled)
                result = onMidi2PerNoteEvent (event);
            break;

        case UmpValues::ChannelVoice::assignablePerNoteController:
            result = onMidi2AssignablePerNoteControllerEvent (event);
            if (result == Result::notHandled)
                result = onMidi2PerNoteEvent (event);
            break;

        case UmpValues::ChannelVoice::registeredController:
            result = onMidi2RegisteredControllerEvent (event);
            if (result == Result::notHandled)
                result = onMidi2ControllerEvent (event);
            break;

        case UmpValues::ChannelVoice::assignableController:
            result = onMidi2AssignableControllerEvent (event);
            if (result == Result::notHandled)
                result = onMidi2ControllerEvent (event);
            break;

        case UmpValues::ChannelVoice::relativeRegisteredController:
            result = onMidi2RelativeRegisteredControllerEvent (event);
            if (result == Result::notHandled)
                result = onMidi2RelativeControllerEvent (event);
            break;

        case UmpValues::ChannelVoice::relativeAssignableController:
            result = onMidi2RelativeAssignableControllerEvent (event);
            if (result == Result::notHandled)
                result = onMidi2RelativeControllerEvent (event);
            break;

        case UmpValues::ChannelVoice::perNotePitchBend:
            result = onMidi2PerNotePitchBendEvent (event);
            break;

        case UmpValues::ChannelVoice::noteOff:
            result = onMidi2NoteOffEvent (event);
            if (result == Result::notHandled)
                result = onMidi2NoteEvent (event);
            break;

        case UmpValues::ChannelVoice::noteOn:
            result = onMidi2NoteOnEvent (event);
            if (result == Result::notHandled)
                result = onMidi2NoteEvent (event);
            break;

        case UmpValues::ChannelVoice::polyPressure:
            result = onMidi2PolyPressureEvent (event);
            break;

        case UmpValues::ChannelVoice::controlChange:
            result = onMidi2ControlChangeEvent (event);
            break;

        case UmpValues::ChannelVoice::programChange:
            result = onMidi2ProgramChangeEvent (event);
            break;

        case UmpValues::ChannelVoice::channelPressure:
            result = onMidi2ChannelPressureEvent (event);
            break;

        case UmpValues::ChannelVoice::pitchBend:
            result = onMidi2PitchBendEvent (event);
            break;

        case UmpValues::ChannelVoice::perNoteManagement:
            result = onMidi2PerNoteManagementEvent (event);
            break;

        default:
            break;
    }

    if (result == Result::notHandled)
        result = onMidi2ChannelVoiceEvent (event);

    if (result == Result::notHandled)
        result = onUmpEvent (event);

    return result;
}
