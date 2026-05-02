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

struct SystemCommonEvent : public UmpEvent
{
    SystemCommonEvent (const UmpEvent& event)
    : UmpEvent (event)
    {
        init ();
    }

    SystemCommonEvent (MidiGroup theGroup, int theStatus)
    : UmpEvent ()
    {
        setattr<uint32_t> (UmpWords::data0Id, 0);
        messageType = MessageTypes::systemCommon;
        userGroup   = theGroup;
        status      = theStatus;
        init ();
    }

    MAKE_BITFIELD (int, group, 0, 4, 24);
    MAKE_BITFIELD (int, status, 0, 8, 16);

    MAKE_COMPUTED_VALUE_MEMBER (
        int, userGroup, [this] () -> int { return group.get () + 1; }, [this] (const int& val) { group = val - 1; });

private:
    void init () { eventName = "System Common"; }
};

struct MidiTimeCodeEvent : public SystemCommonEvent
{
    MidiTimeCodeEvent (const UmpEvent& event)
    : SystemCommonEvent (event)
    {
        init ();
    }

    // theMtcType: which quarter-frame nibble; theMtcData: 0-15 (nibble value).
    MidiTimeCodeEvent (MidiGroup theGroup, MtcMessageType theMtcType, int theMtcData)
    : SystemCommonEvent (theGroup, UmpValues::SystemCommon::midiTimeCode)
    {
        mtcType = theMtcType;
        mtcData = std::clamp (theMtcData, 0, 15);
        init ();
    }

    // data byte 1: 0 | mtcType(3) | mtcData(4), packed at bits [14:8] of word 0
    MAKE_BITFIELD (MtcMessageType, mtcType, 0, 3, 12);
    MAKE_BITFIELD (int,            mtcData, 0, 4, 8);

private:
    void init () { eventName = "System Common: MIDI Time Code"; }
};

struct SongPositionPointerEvent : public SystemCommonEvent
{
    SongPositionPointerEvent (const UmpEvent& event)
    : SystemCommonEvent (event)
    {
        init ();
    }

    // thePosition is a 14-bit beat-pulse offset from the start of the song (0-16383).
    SongPositionPointerEvent (MidiGroup theGroup, MidiWord thePosition)
    : SystemCommonEvent (theGroup, UmpValues::SystemCommon::songPositionPointer)
    {
        lsb = thePosition.getLsb ();
        msb = thePosition.getMsb ();
        init ();
    }

    // LSB is data byte 1 (bits 15-8); MSB is data byte 2 (bits 7-0).
    MAKE_BITFIELD (int, lsb, 0, 7, 8);
    MAKE_BITFIELD (int, msb, 0, 7, 0);

    MAKE_COMPUTED_VALUE_MEMBER (
        int, value, [this] () -> int { return (msb.get () << 7) | lsb.get (); },
        [this] (const int& val)
        {
            MidiWord w { val };
            lsb = w.getLsb ();
            msb = w.getMsb ();
        });

private:
    void init () { eventName = "System Common: Song Position Pointer"; }
};

struct SongSelectEvent : public SystemCommonEvent
{
    SongSelectEvent (const UmpEvent& event)
    : SystemCommonEvent (event)
    {
        init ();
    }

    SongSelectEvent (MidiGroup theGroup, MidiByte theSong)
    : SystemCommonEvent (theGroup, UmpValues::SystemCommon::songSelect)
    {
        song = theSong;
        init ();
    }

    MAKE_BITFIELD (int, song, 0, 7, 8);

private:
    void init () { eventName = "System Common: Song Select"; }
};

struct TuneRequestEvent : public SystemCommonEvent
{
    TuneRequestEvent (const UmpEvent& event)
    : SystemCommonEvent (event)
    {
        init ();
    }

    TuneRequestEvent (MidiGroup theGroup)
    : SystemCommonEvent (theGroup, UmpValues::SystemCommon::tuneRequest)
    {
        init ();
    }

private:
    void init () { eventName = "System Common: Tune Request"; }
};

struct TimingClockEvent : public SystemCommonEvent
{
    TimingClockEvent (const UmpEvent& event)
    : SystemCommonEvent (event)
    {
        init ();
    }

    TimingClockEvent (MidiGroup theGroup)
    : SystemCommonEvent (theGroup, UmpValues::SystemCommon::timingClock)
    {
        init ();
    }

private:
    void init () { eventName = "System Common: Timing Clock"; }
};

struct StartEvent : public SystemCommonEvent
{
    StartEvent (const UmpEvent& event)
    : SystemCommonEvent (event)
    {
        init ();
    }

    StartEvent (MidiGroup theGroup)
    : SystemCommonEvent (theGroup, UmpValues::SystemCommon::start)
    {
        init ();
    }

private:
    void init () { eventName = "System Common: Start"; }
};

struct ContinueEvent : public SystemCommonEvent
{
    ContinueEvent (const UmpEvent& event)
    : SystemCommonEvent (event)
    {
        init ();
    }

    ContinueEvent (MidiGroup theGroup)
    : SystemCommonEvent (theGroup, UmpValues::SystemCommon::continue_)
    {
        init ();
    }

private:
    void init () { eventName = "System Common: Continue"; }
};

struct StopEvent : public SystemCommonEvent
{
    StopEvent (const UmpEvent& event)
    : SystemCommonEvent (event)
    {
        init ();
    }

    StopEvent (MidiGroup theGroup)
    : SystemCommonEvent (theGroup, UmpValues::SystemCommon::stop)
    {
        init ();
    }

private:
    void init () { eventName = "System Common: Stop"; }
};

struct ActiveSensingEvent : public SystemCommonEvent
{
    ActiveSensingEvent (const UmpEvent& event)
    : SystemCommonEvent (event)
    {
        init ();
    }

    ActiveSensingEvent (MidiGroup theGroup)
    : SystemCommonEvent (theGroup, UmpValues::SystemCommon::activeSensing)
    {
        init ();
    }

private:
    void init () { eventName = "System Common: Active Sensing"; }
};

struct SystemResetEvent : public SystemCommonEvent
{
    SystemResetEvent (const UmpEvent& event)
    : SystemCommonEvent (event)
    {
        init ();
    }

    SystemResetEvent (MidiGroup theGroup)
    : SystemCommonEvent (theGroup, UmpValues::SystemCommon::systemReset)
    {
        init ();
    }

private:
    void init () { eventName = "System Common: System Reset"; }
};
