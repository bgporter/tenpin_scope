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

#include "eventFilter.h"

#include "model/ump/channelVoice2.h"
#include "model/ump/umpEvent.h"

EventFilter::EventFilter (AppContext& appContext)
: context { PersistentContext { appContext } }
{
}

bool EventFilter::filterMidiEvent (const UmpEvent& event)
{
    // early exit cases:
    // 1. Are we showing events from this endpoint?
    // 2. Are we filtering on group or channel?

    // !!! implement this based on the filter settings in the context.
    switch (event.messageType.get ())
    {
        case MessageTypes::utility:
            return filterUtility (event);
        case MessageTypes::systemCommon:
            return filterSystemCommon (event);
        case MessageTypes::midi1ChannelVoice:
            [[fallthrough]];
        case MessageTypes::midi2ChannelVoice:
            return filterChannelVoice (event);
        case MessageTypes::sysex7:
            return filterData7 (event);
        case MessageTypes::sysex8:
            return filterData8 (event);
        case MessageTypes::flexData:
            return filterFlexData (event);
        case MessageTypes::stream:
            return filterStream (event);
        case MessageTypes::reserved_6:
            [[fallthrough]];
        case MessageTypes::reserved_7:
            [[fallthrough]];
        case MessageTypes::reserved_8:
            [[fallthrough]];
        case MessageTypes::reserved_9:
            [[fallthrough]];
        case MessageTypes::reserved_a:
            [[fallthrough]];
        case MessageTypes::reserved_b:
            return filterUndefined (event);
        default:
            jassertfalse; // this should never happen, but if it does, we want to know about it.
            break;
    }
    return true;
}

bool EventFilter::filterUtility (const UmpEvent& event)
{
    return context.showUtility;
}

bool EventFilter::filterSystemCommon (const UmpEvent& event)
{
    return context.showSystemCommonRealtime;
}

bool EventFilter::filterChannelVoice (const UmpEvent& event)
{
    // exit early if we're not showing any channel voice events at all.
    if (!context.showChannelVoice)
        return false;

    const Midi2ChannelVoiceEvent cvEvent { event };
    switch (cvEvent.status.get ())
    {
        // NOTE ON / OFF
        case UmpValues::ChannelVoice::noteOff:
            [[fallthrough]];
        case UmpValues::ChannelVoice::noteOn:
            return context.showNoteOnOff;

        // ALL THE PER-NOTE EVENTS
        case UmpValues::ChannelVoice::polyPressure:
            [[fallthrough]];
        case UmpValues::ChannelVoice::perNotePitchBend:
            [[fallthrough]];
        case UmpValues::ChannelVoice::perNoteManagement:
            [[fallthrough]];
        case UmpValues::ChannelVoice::registeredPerNoteController:
            [[fallthrough]];
        case UmpValues::ChannelVoice::assignablePerNoteController:
            return context.showPerNoteEvents;

        // ALL THE CHANNEL CONTROLLER EVENTS
        case UmpValues::ChannelVoice::controlChange:
            [[fallthrough]];
        case UmpValues::ChannelVoice::registeredController:
            [[fallthrough]];
        case UmpValues::ChannelVoice::assignableController:
            [[fallthrough]];
        case UmpValues::ChannelVoice::relativeRegisteredController:
            [[fallthrough]];
        case UmpValues::ChannelVoice::relativeAssignableController:
            return context.showControlChange;

        // PROGRAM CHANGE
        case UmpValues::ChannelVoice::programChange:
            return context.showProgramChange;

        // CHANNEL PRESSURE
        case UmpValues::ChannelVoice::channelPressure:
            return context.showChannelPressure;

        // PITCH BEND
        case UmpValues::ChannelVoice::pitchBend:
            return context.showPitchBend;
        default:
            jassertfalse;
            return true;
    }
}

bool EventFilter::filterData7 (const UmpEvent& event)
{
    return context.showSysex7;
}

bool EventFilter::filterData8 (const UmpEvent& event)
{
    return context.showSysex8Packets;
}

bool EventFilter::filterFlexData (const UmpEvent& event)
{
    return context.showFlexData;
}

bool EventFilter::filterStream (const UmpEvent& event)
{
    return context.showStreamData;
}

bool EventFilter::filterUndefined (const UmpEvent& event)
{
    return context.showUndefined;
}

#if RUN_UNIT_TESTS
#include "test/test_EventFilter.inl"
#endif
