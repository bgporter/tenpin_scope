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

#include "model/ump/channelVoice2.h"

UmpHandler::UmpHandler () {}

UmpHandler::~UmpHandler () {}

UmpHandler::Result UmpHandler::handle (const UmpEvent& event)
{
    Result result { preDispatch (event) };
    if (result == Result::ok)
    {
        switch (event.messageType)
        {
            case MessageTypes::midi2ChannelVoice:
                result = handleMidi2ChannelVoiceEvent (event);
                break;
            default:
                result = onUmpEvent (event);
                break;
        }
    }
    return postDispatch (result);
}

UmpHandler::Result UmpHandler::handleMidi2ChannelVoiceEvent (const UmpEvent& event)
{
    Result result { defaultResult };
    Midi2ChannelVoiceEvent midi2ChannelVoiceEvent (event);
    switch (midi2ChannelVoiceEvent.status)
    {
        case UmpValues::registeredPerNoteController:
            result = onMidi2RegisteredPerNoteControllerEvent (event);
            if (result == Result::notHandled)
                result = onMidi2PerNoteEvent (event);
            break;

        case UmpValues::assignablePerNoteController:
            result = onMidi2AssignablePerNoteControllerEvent (event);
            if (result == Result::notHandled)
                result = onMidi2PerNoteEvent (event);
            break;

        case UmpValues::registeredController:
            result = onMidi2RegisteredControllerEvent (event);
            if (result == Result::notHandled)
                result = onMidi2ControllerEvent (event);
            break;

        case UmpValues::assignableController:
            result = onMidi2AssignableControllerEvent (event);
            if (result == Result::notHandled)
                result = onMidi2ControllerEvent (event);
            break;

        case UmpValues::relativeRegisteredController:
            result = onMidi2RelativeRegisteredControllerEvent (event);
            if (result == Result::notHandled)
                result = onMidi2RelativeControllerEvent (event);
            break;

        case UmpValues::relativeAssignableController:
            result = onMidi2RelativeAssignableControllerEvent (event);
            if (result == Result::notHandled)
                result = onMidi2RelativeControllerEvent (event);
            break;

        case UmpValues::perNotePitchBend:
            result = onMidi2PerNotePitchBendEvent (event);
            break;

        case UmpValues::noteOff:
            result = onMidi2NoteOffEvent (event);
            if (result == Result::notHandled)
                result = onMidi2NoteEvent (event);
            break;

        case UmpValues::noteOn:
            result = onMidi2NoteOnEvent (event);
            if (result == Result::notHandled)
                result = onMidi2NoteEvent (event);
            break;

        case UmpValues::polyPressure:
            result = onMidi2PolyPressureEvent (event);
            break;

        case UmpValues::controlChange:
            result = onMidi2ControlChangeEvent (event);
            break;

        case UmpValues::programChange:
            result = onMidi2ProgramChangeEvent (event);
            break;

        case UmpValues::channelPressure:
            result = onMidi2ChannelPressureEvent (event);
            break;

        case UmpValues::pitchBend:
            result = onMidi2PitchBendEvent (event);
            break;

        case UmpValues::perNoteManagement:
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
