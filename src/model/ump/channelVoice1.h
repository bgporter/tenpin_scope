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

struct Midi1ChannelVoiceEvent : public UmpEvent
{
    Midi1ChannelVoiceEvent (const UmpEvent& event)
    : UmpEvent (event)
    {
        init ();
    }

    Midi1ChannelVoiceEvent (MidiGroup theGroup, MidiNibble theStatus, MidiChannel theChannel)
    : UmpEvent ()
    {
        setattr<uint32_t> (UmpWords::data0Id, 0);
        messageType = MessageTypes::midi1ChannelVoice;
        userGroup   = theGroup;
        status      = theStatus;
        userChannel = theChannel;
        init ();
    }

    MAKE_BITFIELD (int, group, 0, 4, 24);
    MAKE_BITFIELD (int, status, 0, 4, 20);
    MAKE_BITFIELD (int, channel, 0, 4, 16);

    MAKE_COMPUTED_VALUE_MEMBER (
        int, userGroup, [this] () -> int { return group.get () + 1; }, [this] (const int& val) { group = val - 1; });

    MAKE_COMPUTED_VALUE_MEMBER (
        int, userChannel, [this] () -> int { return channel.get () + 1; },
        [this] (const int& val) { channel = val - 1; });

private:
    void init () { eventName = "MIDI 1 Channel Voice"; }
};

struct Midi1NoteEvent : public Midi1ChannelVoiceEvent
{
    Midi1NoteEvent (const UmpEvent& event)
    : Midi1ChannelVoiceEvent (event)
    {
        init ();
    }

    Midi1NoteEvent (MidiGroup theGroup, MidiNibble theStatus, MidiChannel theChannel, MidiByte theNote,
                    MidiByte theVelocity)
    : Midi1ChannelVoiceEvent (theGroup, theStatus, theChannel)
    {
        note     = theNote;
        velocity = theVelocity;
        init ();
    }

    MAKE_BITFIELD (int, note, 0, 7, 8);
    MAKE_BITFIELD (int, velocity, 0, 7, 0);

private:
    void init () { eventName = "MIDI 1 Note"; }
};

struct Midi1NoteOffEvent : public Midi1NoteEvent
{
    Midi1NoteOffEvent (const UmpEvent& event)
    : Midi1NoteEvent (event)
    {
        init ();
    }

    Midi1NoteOffEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theNote, MidiByte theVelocity)
    : Midi1NoteEvent (theGroup, UmpValues::ChannelVoice::noteOff, theChannel, theNote, theVelocity)
    {
        init ();
    }

private:
    void init () { eventName = "MIDI 1 Note Off"; }
};

struct Midi1NoteOnEvent : public Midi1NoteEvent
{
    Midi1NoteOnEvent (const UmpEvent& event)
    : Midi1NoteEvent (event)
    {
        init ();
    }

    Midi1NoteOnEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theNote, MidiByte theVelocity)
    : Midi1NoteEvent (theGroup, UmpValues::ChannelVoice::noteOn, theChannel, theNote, theVelocity)
    {
        init ();
    }

private:
    void init () { eventName = "MIDI 1 Note On"; }
};

struct Midi1PolyPressureEvent : public Midi1ChannelVoiceEvent
{
    Midi1PolyPressureEvent (const UmpEvent& event)
    : Midi1ChannelVoiceEvent (event)
    {
        init ();
    }

    Midi1PolyPressureEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theNote, MidiByte thePressure)
    : Midi1ChannelVoiceEvent (theGroup, UmpValues::ChannelVoice::polyPressure, theChannel)
    {
        note     = theNote;
        pressure = thePressure;
        init ();
    }

    MAKE_BITFIELD (int, note, 0, 7, 8);
    MAKE_BITFIELD (int, pressure, 0, 7, 0);

private:
    void init () { eventName = "MIDI 1 Poly Pressure"; }
};

struct Midi1ControlChangeEvent : public Midi1ChannelVoiceEvent
{
    Midi1ControlChangeEvent (const UmpEvent& event)
    : Midi1ChannelVoiceEvent (event)
    {
        init ();
    }

    Midi1ControlChangeEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theController, MidiByte theValue)
    : Midi1ChannelVoiceEvent (theGroup, UmpValues::ChannelVoice::controlChange, theChannel)
    {
        controller = theController;
        value      = theValue;
        init ();
    }

    MAKE_BITFIELD (int, controller, 0, 7, 8);
    MAKE_BITFIELD (int, value, 0, 7, 0);

private:
    void init () { eventName = "MIDI 1 Control Change"; }
};

struct Midi1ProgramChangeEvent : public Midi1ChannelVoiceEvent
{
    Midi1ProgramChangeEvent (const UmpEvent& event)
    : Midi1ChannelVoiceEvent (event)
    {
        init ();
    }

    Midi1ProgramChangeEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theProgram)
    : Midi1ChannelVoiceEvent (theGroup, UmpValues::ChannelVoice::programChange, theChannel)
    {
        program = theProgram;
        init ();
    }

    MAKE_BITFIELD (int, program, 0, 7, 8);

private:
    void init () { eventName = "MIDI 1 Program Change"; }
};

struct Midi1ChannelPressureEvent : public Midi1ChannelVoiceEvent
{
    Midi1ChannelPressureEvent (const UmpEvent& event)
    : Midi1ChannelVoiceEvent (event)
    {
        init ();
    }

    Midi1ChannelPressureEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte thePressure)
    : Midi1ChannelVoiceEvent (theGroup, UmpValues::ChannelVoice::channelPressure, theChannel)
    {
        pressure = thePressure;
        init ();
    }

    MAKE_BITFIELD (int, pressure, 0, 7, 8);

private:
    void init () { eventName = "MIDI 1 Channel Pressure"; }
};

struct Midi1PitchBendEvent : public Midi1ChannelVoiceEvent
{
    Midi1PitchBendEvent (const UmpEvent& event)
    : Midi1ChannelVoiceEvent (event)
    {
        init ();
    }

    // theValue is a 14-bit pitch bend value (0-16383, center at 8192).
    Midi1PitchBendEvent (MidiGroup theGroup, MidiChannel theChannel, MidiWord theValue)
    : Midi1ChannelVoiceEvent (theGroup, UmpValues::ChannelVoice::pitchBend, theChannel)
    {
        lsb = theValue.getLsb ();
        msb = theValue.getMsb ();
        init ();
    }

    // LSB is the first data byte (bits 15-8); MSB is the second (bits 7-0).
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

    // Bipolar representation: 0 (raw) = -8192, 8192 (raw) = 0, 16383 (raw) = +8191.
    MAKE_COMPUTED_VALUE_MEMBER (
        int, userValue, [this] () -> int { return value.get () - 8192; },
        [this] (const int& val) { value = std::clamp (val + 8192, 0, 16383); });

    MAKE_COMPUTED_VALUE_MEMBER (
        float, userFloat, [this] () -> float { return static_cast<float> (userValue.get ()) / 8192.0f; },
        [this] (const float& val) { userValue = static_cast<int> (std::round (std::clamp (val, -1.0f, 1.0f) * 8192.0f)); });

private:
    void init () { eventName = "MIDI 1 Pitch Bend"; }
};
