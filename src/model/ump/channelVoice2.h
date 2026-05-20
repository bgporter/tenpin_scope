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

struct Midi2ChannelVoiceEvent : public UmpEvent
{
    Midi2ChannelVoiceEvent (const UmpEvent& event)
    : UmpEvent (event)
    {
        init ();
    }

    Midi2ChannelVoiceEvent (MidiGroup theGroup, MidiNibble theStatus, MidiChannel theChannel)
    : UmpEvent ()
    {
        // we need to add the properties for the raw data words to the value tree.
        setattr<uint32_t> (UmpWords::data0Id, 0);
        setattr<uint32_t> (UmpWords::data1Id, 0);
        messageType = MessageTypes::midi2ChannelVoice;
        userGroup   = theGroup;
        status      = theStatus;
        userChannel = theChannel;
        init ();
    }

    MAKE_BITFIELD (int, status, 0, 4, 20);

private:
    void init () { eventName = "MIDI 2 Channel Voice"; }
};

struct Midi2PerNoteEvent : public Midi2ChannelVoiceEvent
{
    Midi2PerNoteEvent (const UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
        init ();
    }

    Midi2PerNoteEvent (MidiGroup theGroup, MidiNibble theStatus, MidiChannel theChannel, MidiByte theNote,
                       uint8_t theController, uint32_t theValue)
    : Midi2ChannelVoiceEvent (theGroup, theStatus, theChannel)
    {
        note       = theNote;
        controller = theController;
        setattr<uint32_t> (UmpWords::data1Id, theValue);
        init ();
    }

    Midi2PerNoteEvent (MidiGroup theGroup, MidiNibble theStatus, MidiChannel theChannel, MidiByte theNote,
                       uint8_t theController, MidiUnipolarFloat theValue)
    : Midi2PerNoteEvent (theGroup, theStatus, theChannel, theNote, theController, theValue.toUint32 ())
    {
    }

    MAKE_BITFIELD (int, note, 0, 8, 8);
    MAKE_BITFIELD (int, controller, 0, 8, 0);
    MAKE_BITFIELD (uint32_t, value, 1, 32, 0);

    MAKE_COMPUTED_VALUE_MEMBER (
        float, valueFloat, [this] () -> float { return MidiUnipolarFloat::fromUint32 (value.get ()); },
        [this] (const MidiUnipolarFloat& val) { value = val.toUint32 (); });

private:
    void init () { eventName = "MIDI 2 Per Note"; }
};

struct Midi2PerNotePitchBendEvent : public Midi2ChannelVoiceEvent
{
    Midi2PerNotePitchBendEvent (const UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
        init ();
    }

    Midi2PerNotePitchBendEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theNote, int32_t theValue)
    : Midi2ChannelVoiceEvent (theGroup, UmpValues::ChannelVoice::perNotePitchBend, theChannel)
    {
        note  = theNote;
        value = theValue;
        init ();
    }

    Midi2PerNotePitchBendEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theNote, MidiBipolarFloat theValue)
    : Midi2PerNotePitchBendEvent (theGroup, theChannel, theNote, theValue.toInt32 ())
    {
    }

    MAKE_BITFIELD (int, note, 0, 8, 8);
    MAKE_BITFIELD (int32_t, value, 1, 32, 0);

    MAKE_COMPUTED_VALUE_MEMBER (
        float, valueFloat, [this] () -> float { return MidiBipolarFloat::fromInt32 (value.get ()); },
        [this] (const MidiBipolarFloat& val) { value = val.toInt32 (); });

private:
    void init () { eventName = "MIDI 2 Per Note Pitch Bend"; }
};

struct Midi2ControlChangeEvent : public Midi2ChannelVoiceEvent
{
    Midi2ControlChangeEvent (const UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
        init ();
    }

    Midi2ControlChangeEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theController, uint32_t theValue)
    : Midi2ChannelVoiceEvent (theGroup, UmpValues::ChannelVoice::controlChange, theChannel)
    {
        controller = theController;
        value      = theValue;
        init ();
    }

    Midi2ControlChangeEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theController,
                             MidiUnipolarFloat theValue)
    : Midi2ControlChangeEvent (theGroup, theChannel, theController, theValue.toUint32 ())
    {
    }

    MAKE_BITFIELD (int, controller, 0, 8, 8);
    MAKE_BITFIELD (uint32_t, value, 1, 32, 0);

    MAKE_COMPUTED_VALUE_MEMBER (
        float, valueFloat, [this] () -> float { return MidiUnipolarFloat::fromUint32 (value.get ()); },
        [this] (const MidiUnipolarFloat& val) { value = val.toUint32 (); });

private:
    void init () { eventName = "MIDI 2 Control Change"; }
};

struct Midi2ProgramChangeEvent : public Midi2ChannelVoiceEvent
{
    Midi2ProgramChangeEvent (const UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
        init ();
    }

    Midi2ProgramChangeEvent (MidiGroup theGroup, MidiChannel theChannel, MidiWord theBank, MidiByte theProgram)
    : Midi2ChannelVoiceEvent (theGroup, UmpValues::ChannelVoice::programChange, theChannel)
    {
        bankValid = true;
        bank      = theBank;
        program   = theProgram;
        init ();
    }

    Midi2ProgramChangeEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theProgram)
    : Midi2ChannelVoiceEvent (theGroup, UmpValues::ChannelVoice::programChange, theChannel)
    {
        bankValid = false;
        bankMsb   = 0;
        bankLsb   = 0;
        program   = theProgram;
        init ();
    }

    MAKE_BITFIELD (bool, bankValid, 0, 1, 0);
    MAKE_BITFIELD (int, program, 1, 7, 24);
    MAKE_BITFIELD (int, bankMsb, 1, 7, 8);
    MAKE_BITFIELD (int, bankLsb, 1, 7, 0);

    MAKE_COMPUTED_VALUE_MEMBER (
        int, bank, [this] () -> int { return (bankMsb.get () << 7) | bankLsb.get (); },
        [this] (const int& val)
        {
            MidiWord w { val };
            bankMsb = w.getMsb ();
            bankLsb = w.getLsb ();
        });

private:
    void init () { eventName = "MIDI 2 Program Change"; }
};

struct Midi2PerNoteManagementEvent : public Midi2ChannelVoiceEvent
{
    Midi2PerNoteManagementEvent (const UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
        init ();
    }

    Midi2PerNoteManagementEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theNote, bool theDetach,
                                 bool theReset)
    : Midi2ChannelVoiceEvent (theGroup, UmpValues::ChannelVoice::perNoteManagement, theChannel)
    {
        jassert (theDetach || theReset);
        note   = theNote;
        detach = theDetach;
        reset  = theReset;
        init ();
    }

    MAKE_BITFIELD (int, note, 0, 8, 8);
    MAKE_BITFIELD (bool, detach, 0, 1, 1);
    MAKE_BITFIELD (bool, reset, 0, 1, 0);

private:
    void init () { eventName = "MIDI 2 Per Note Management"; }
};

struct Midi2PolyPressureEvent : public Midi2ChannelVoiceEvent
{
    Midi2PolyPressureEvent (const UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
        init ();
    }

    Midi2PolyPressureEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theNote, uint32_t theValue)
    : Midi2ChannelVoiceEvent (theGroup, UmpValues::ChannelVoice::polyPressure, theChannel)
    {
        note  = theNote;
        value = theValue;
        init ();
    }

    Midi2PolyPressureEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theNote, MidiUnipolarFloat theValue)
    : Midi2PolyPressureEvent (theGroup, theChannel, theNote, theValue.toUint32 ())
    {
    }

    MAKE_BITFIELD (int, note, 0, 8, 8);
    MAKE_BITFIELD (uint32_t, value, 1, 32, 0);

    MAKE_COMPUTED_VALUE_MEMBER (
        float, valueFloat, [this] () -> float { return MidiUnipolarFloat::fromUint32 (value.get ()); },
        [this] (const MidiUnipolarFloat& val) { value = val.toUint32 (); });

private:
    void init () { eventName = "MIDI 2 Poly Pressure"; }
};

struct Midi2ChannelPressureEvent : public Midi2ChannelVoiceEvent
{
    Midi2ChannelPressureEvent (const UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
        init ();
    }

    Midi2ChannelPressureEvent (MidiGroup theGroup, MidiChannel theChannel, uint32_t theValue)
    : Midi2ChannelVoiceEvent (theGroup, UmpValues::ChannelVoice::channelPressure, theChannel)
    {
        value = theValue;
        init ();
    }

    Midi2ChannelPressureEvent (MidiGroup theGroup, MidiChannel theChannel, MidiUnipolarFloat theValue)
    : Midi2ChannelPressureEvent (theGroup, theChannel, theValue.toUint32 ())
    {
    }

    MAKE_BITFIELD (uint32_t, value, 1, 32, 0);

    MAKE_COMPUTED_VALUE_MEMBER (
        float, valueFloat, [this] () -> float { return MidiUnipolarFloat::fromUint32 (value.get ()); },
        [this] (const MidiUnipolarFloat& val) { value = val.toUint32 (); });

private:
    void init () { eventName = "MIDI 2 Channel Pressure"; }
};

struct Midi2PitchBendEvent : public Midi2ChannelVoiceEvent
{
    Midi2PitchBendEvent (const UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
        init ();
    }

    Midi2PitchBendEvent (MidiGroup theGroup, MidiChannel theChannel, int32_t theValue)
    : Midi2ChannelVoiceEvent (theGroup, UmpValues::ChannelVoice::pitchBend, theChannel)
    {
        value = theValue;
        init ();
    }

    Midi2PitchBendEvent (MidiGroup theGroup, MidiChannel theChannel, MidiBipolarFloat theValue)
    : Midi2PitchBendEvent (theGroup, theChannel, theValue.toInt32 ())
    {
    }

    MAKE_BITFIELD (int32_t, value, 1, 32, 0);

    MAKE_COMPUTED_VALUE_MEMBER (
        float, valueFloat, [this] () -> float { return MidiBipolarFloat::fromInt32 (value.get ()); },
        [this] (const MidiBipolarFloat& val) { value = val.toInt32 (); });

private:
    void init () { eventName = "MIDI 2 Pitch Bend"; }
};

struct Midi2RegisteredPerNoteControllerEvent : public Midi2PerNoteEvent
{
    Midi2RegisteredPerNoteControllerEvent (const UmpEvent& event)
    : Midi2PerNoteEvent (event)
    {
        init ();
    }

    Midi2RegisteredPerNoteControllerEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theNote,
                                           uint8_t theController, uint32_t theValue)
    : Midi2PerNoteEvent (theGroup, UmpValues::ChannelVoice::registeredPerNoteController, theChannel, theNote, theController, theValue)
    {
        init ();
    }

    Midi2RegisteredPerNoteControllerEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theNote,
                                           uint8_t theController, MidiUnipolarFloat theValue)
    : Midi2RegisteredPerNoteControllerEvent (theGroup, theChannel, theNote, theController, theValue.toUint32 ())
    {
    }

private:
    void init () { eventName = "MIDI 2 Registered Per Note Controller"; }
};

struct Midi2AssignablePerNoteControllerEvent : public Midi2PerNoteEvent
{
    Midi2AssignablePerNoteControllerEvent (const UmpEvent& event)
    : Midi2PerNoteEvent (event)
    {
        init ();
    }

    Midi2AssignablePerNoteControllerEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theNote,
                                           uint8_t theController, uint32_t theValue)
    : Midi2PerNoteEvent (theGroup, UmpValues::ChannelVoice::assignablePerNoteController, theChannel, theNote, theController, theValue)
    {
        init ();
    }

    Midi2AssignablePerNoteControllerEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theNote,
                                           uint8_t theController, MidiUnipolarFloat theValue)
    : Midi2AssignablePerNoteControllerEvent (theGroup, theChannel, theNote, theController, theValue.toUint32 ())
    {
    }

private:
    void init () { eventName = "MIDI 2 Assignable Per Note Controller"; }
};

struct Midi2ControllerEvent : public Midi2ChannelVoiceEvent
{
    Midi2ControllerEvent (const UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
        init ();
    }

    Midi2ControllerEvent (MidiGroup theGroup, MidiNibble theStatus, MidiChannel theChannel, MidiByte theBank,
                          uint8_t theController, uint32_t theValue)
    : Midi2ChannelVoiceEvent (theGroup, theStatus, theChannel)
    {
        bank       = theBank;
        controller = theController;
        setattr<uint32_t> (UmpWords::data1Id, theValue);
        init ();
    }

    Midi2ControllerEvent (MidiGroup theGroup, MidiNibble theStatus, MidiChannel theChannel, MidiByte theBank,
                          uint8_t theController, MidiUnipolarFloat theValue)
    : Midi2ControllerEvent (theGroup, theStatus, theChannel, theBank, theController, theValue.toUint32 ())
    {
    }

    MAKE_BITFIELD (int, bank, 0, 8, 8);
    MAKE_BITFIELD (int, controller, 0, 8, 0);
    MAKE_BITFIELD (uint32_t, value, 1, 32, 0);

    MAKE_COMPUTED_VALUE_MEMBER (
        float, valueFloat, [this] () -> float { return MidiUnipolarFloat::fromUint32 (value.get ()); },
        [this] (const MidiUnipolarFloat& val) { value = val.toUint32 (); });

private:
    void init () { eventName = "MIDI 2 Control Change"; }
};

struct Midi2RegisteredControllerEvent : public Midi2ControllerEvent
{
    Midi2RegisteredControllerEvent (const UmpEvent& event)
    : Midi2ControllerEvent (event)
    {
        init ();
    }

    Midi2RegisteredControllerEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theBank,
                                    uint8_t theController, uint32_t theValue)
    : Midi2ControllerEvent (theGroup, UmpValues::ChannelVoice::registeredController, theChannel, theBank, theController, theValue)
    {
        init ();
    }

    Midi2RegisteredControllerEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theBank,
                                    uint8_t theController, MidiUnipolarFloat theValue)
    : Midi2RegisteredControllerEvent (theGroup, theChannel, theBank, theController, theValue.toUint32 ())
    {
    }

private:
    void init () { eventName = "MIDI 2 Registered Controller"; }
};

struct Midi2AssignableControllerEvent : public Midi2ControllerEvent
{
    Midi2AssignableControllerEvent (const UmpEvent& event)
    : Midi2ControllerEvent (event)
    {
        init ();
    }

    Midi2AssignableControllerEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theBank,
                                    uint8_t theController, uint32_t theValue)
    : Midi2ControllerEvent (theGroup, UmpValues::ChannelVoice::assignableController, theChannel, theBank, theController, theValue)
    {
        init ();
    }

    Midi2AssignableControllerEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theBank,
                                    uint8_t theController, MidiUnipolarFloat theValue)
    : Midi2AssignableControllerEvent (theGroup, theChannel, theBank, theController, theValue.toUint32 ())
    {
    }

private:
    void init () { eventName = "MIDI 2 Assignable Controller"; }
};

struct Midi2RelativeControllerEvent : public Midi2ChannelVoiceEvent
{
    Midi2RelativeControllerEvent (const UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
        init ();
    }

    Midi2RelativeControllerEvent (MidiGroup theGroup, MidiNibble theStatus, MidiChannel theChannel, MidiByte theBank,
                                  uint8_t theController, int32_t theValue)
    : Midi2ChannelVoiceEvent (theGroup, theStatus, theChannel)
    {
        bank       = theBank;
        controller = theController;
        setattr<int32_t> (UmpWords::data1Id, theValue);
        init ();
    }

    Midi2RelativeControllerEvent (MidiGroup theGroup, MidiNibble theStatus, MidiChannel theChannel, MidiByte theBank,
                                  uint8_t theController, MidiBipolarFloat theValue)
    : Midi2RelativeControllerEvent (theGroup, theStatus, theChannel, theBank, theController, theValue.toInt32 ())
    {
    }

    MAKE_BITFIELD (int, bank, 0, 8, 8);
    MAKE_BITFIELD (int, controller, 0, 8, 0);
    MAKE_BITFIELD (int32_t, value, 1, 32, 0);

    MAKE_COMPUTED_VALUE_MEMBER (
        float, valueFloat, [this] () -> float { return MidiBipolarFloat::fromInt32 (value.get ()); },
        [this] (const MidiBipolarFloat& val) { value = val.toInt32 (); });

private:
    void init () { eventName = "MIDI 2 Relative Control Change"; }
};

struct Midi2RelativeRegisteredControllerEvent : public Midi2RelativeControllerEvent
{
    Midi2RelativeRegisteredControllerEvent (const UmpEvent& event)
    : Midi2RelativeControllerEvent (event)
    {
        init ();
    }

    Midi2RelativeRegisteredControllerEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theBank,
                                            uint8_t theController, int32_t theValue)
    : Midi2RelativeControllerEvent (theGroup, UmpValues::ChannelVoice::relativeRegisteredController, theChannel, theBank,
                                    theController, theValue)
    {
        init ();
    }

    Midi2RelativeRegisteredControllerEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theBank,
                                            uint8_t theController, MidiBipolarFloat theValue)
    : Midi2RelativeRegisteredControllerEvent (theGroup, theChannel, theBank, theController, theValue.toInt32 ())
    {
    }

private:
    void init () { eventName = "MIDI 2 Relative Registered Controller"; }
};

struct Midi2RelativeAssignableControllerEvent : public Midi2RelativeControllerEvent
{
    Midi2RelativeAssignableControllerEvent (const UmpEvent& event)
    : Midi2RelativeControllerEvent (event)
    {
        init ();
    }

    Midi2RelativeAssignableControllerEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theBank,
                                            uint8_t theController, int32_t theValue)
    : Midi2RelativeControllerEvent (theGroup, UmpValues::ChannelVoice::relativeAssignableController, theChannel, theBank,
                                    theController, theValue)
    {
        init ();
    }

    Midi2RelativeAssignableControllerEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theBank,
                                            uint8_t theController, MidiBipolarFloat theValue)
    : Midi2RelativeAssignableControllerEvent (theGroup, theChannel, theBank, theController, theValue.toInt32 ())
    {
    }

private:
    void init () { eventName = "MIDI 2 Relative Assignable Controller"; }
};

struct Midi2NoteEvent : public Midi2ChannelVoiceEvent
{
    Midi2NoteEvent (const UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
        init ();
    }

    Midi2NoteEvent (MidiGroup theGroup, MidiNibble theStatus, MidiChannel theChannel, MidiByte theNote,
                    uint16_t theVelocity)
    : Midi2ChannelVoiceEvent (theGroup, theStatus, theChannel)
    {
        note     = theNote;
        velocity = theVelocity;
        init ();
    }

    Midi2NoteEvent (MidiGroup theGroup, MidiNibble theStatus, MidiChannel theChannel, MidiByte theNote,
                    MidiUnipolarFloat theVelocity)
    : Midi2NoteEvent (theGroup, theStatus, theChannel, theNote, theVelocity.toUint16 ())
    {
    }

    MAKE_BITFIELD (int, note, 0, 8, 8);
    MAKE_BITFIELD (int, attributeType, 0, 8, 0);
    MAKE_BITFIELD (int, velocity, 1, 16, 16);
    MAKE_COMPUTED_VALUE_MEMBER (
        float, velocityFloat, [this] () -> float { return velocity.get () / 65535.0f; },
        [this] (const MidiUnipolarFloat& val) { velocity = static_cast<uint16_t> (0.5f + val * 65535.0f); });
    MAKE_BITFIELD (int, attributeValue, 1, 16, 0);

private:
    void init () { eventName = "MIDI 2 Note"; }
};

struct Midi2NoteOffEvent : public Midi2NoteEvent
{
    Midi2NoteOffEvent (const UmpEvent& event)
    : Midi2NoteEvent (event)
    {
        init ();
    }

    Midi2NoteOffEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theNote, MidiUnipolarFloat theVelocity)
    : Midi2NoteOffEvent (theGroup, theChannel, theNote, theVelocity.toUint16 ())
    {
    }

    Midi2NoteOffEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theNote, uint16_t theVelocity)
    : Midi2NoteEvent (theGroup, UmpValues::ChannelVoice::noteOff, theChannel, theNote, theVelocity)
    {
        init ();
    }

private:
    void init () { eventName = "MIDI 2 Note Off"; }
};

struct Midi2NoteOnEvent : public Midi2NoteEvent
{
    Midi2NoteOnEvent (const UmpEvent& event)
    : Midi2NoteEvent (event)
    {
        init ();
    }

    Midi2NoteOnEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theNote, MidiUnipolarFloat theVelocity)
    : Midi2NoteOnEvent (theGroup, theChannel, theNote, theVelocity.toUint16 ())
    {
    }

    Midi2NoteOnEvent (MidiGroup theGroup, MidiChannel theChannel, MidiByte theNote, uint16_t theVelocity)
    : Midi2NoteEvent (theGroup, UmpValues::ChannelVoice::noteOn, theChannel, theNote, theVelocity)
    {
        init ();
    }

private:
    void init () { eventName = "MIDI 2 Note On"; }
};
