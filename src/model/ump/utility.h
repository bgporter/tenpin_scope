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

#include "../midiTypes.h"
#include "umpEvent.h"

struct UtilityEvent : public UmpEvent
{
    UtilityEvent (const UmpEvent& event)
    : UmpEvent (event)
    {
        eventName = "Utility";
    }

    UtilityEvent (MidiNibble theStatus)
    {
        eventName = "Utility";
        setattr<uint32_t> (UmpWords::data0Id, 0);
        status = theStatus;
    }

    MAKE_BITFIELD (int, status, 0, 4, 20);

private:
    using UmpEvent::group;
    using UmpEvent::groupId;
    using UmpEvent::userGroup;
    using UmpEvent::userGroupId;
    using UmpEvent::channel;
    using UmpEvent::channelId;
    using UmpEvent::userChannel;
    using UmpEvent::userChannelId;
};

struct NoOpEvent : public UtilityEvent
{
    NoOpEvent (const UmpEvent& event)
    : UtilityEvent (event)
    {
        init ();
    }

    NoOpEvent ()
    : UtilityEvent (UmpValues::Utility::noop)
    {
        init ();
    }

private:
    void init () { eventName = "Utility: NoOp"; }
};

// Shared base for utility events that carry a 16-bit timestamp field.
struct UtilityEvent16T : public UtilityEvent
{
    using UtilityEvent::UtilityEvent;
    MAKE_BITFIELD (int, ticks, 0, 16, 0);
};

struct JrClockEvent : public UtilityEvent16T
{
    JrClockEvent (const UmpEvent& event)
    : UtilityEvent16T (event)
    {
        init ();
    }

    JrClockEvent (int timestampTicks)
    : UtilityEvent16T (UmpValues::Utility::jrClock)
    {
        init ();
        ticks = std::clamp (timestampTicks, 0, 0xFFFF);
    }

private:
    void init () { eventName = "Utility: JR Clock"; }
};

struct JrTimestampEvent : public UtilityEvent16T
{
    JrTimestampEvent (const UmpEvent& event)
    : UtilityEvent16T (event)
    {
        init ();
    }

    JrTimestampEvent (int timestampTicks)
    : UtilityEvent16T (UmpValues::Utility::jrTimestamp)
    {
        init ();
        ticks = std::clamp (timestampTicks, 0, 0xFFFF);
    }

private:
    void init () { eventName = "Utility: JR Timestamp"; }
};

struct DeltaTicksPerQuarterEvent : public UtilityEvent16T
{
    DeltaTicksPerQuarterEvent (const UmpEvent& event)
    : UtilityEvent16T (event)
    {
        init ();
    }

    DeltaTicksPerQuarterEvent (int ticksPerQuarter)
    : UtilityEvent16T (UmpValues::Utility::deltaClockstampTPQ)
    {
        init ();
        ticks = std::clamp (ticksPerQuarter, 0, 0xFFFF);
    }

private:
    void init () { eventName = "Utility: Delta Ticks Per Quarter"; }
};

struct DeltaTicksSinceLastEvent : public UtilityEvent
{
    DeltaTicksSinceLastEvent (const UmpEvent& event)
    : UtilityEvent (event)
    {
        init ();
    }

    DeltaTicksSinceLastEvent (int ticksSinceLastEvent)
    : UtilityEvent (UmpValues::Utility::deltaClockstampSinceLastEvent)
    {
        init ();
        ticks = ticksSinceLastEvent;
    }

    MAKE_BITFIELD (int, ticks, 0, 20, 0);

private:
    void init () { eventName = "Utility: Delta Ticks Since Last Event"; }
};
