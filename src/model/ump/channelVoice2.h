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

#include "../midiTypes.h"
#include "umpEvent.h"

struct Midi2ChannelVoiceEvent : public UmpEvent
{
    Midi2ChannelVoiceEvent (UmpEvent& event)
    : UmpEvent (static_cast<juce::ValueTree> (event))
    {
    }

    Midi2ChannelVoiceEvent (MidiNibble theGroup, MidiNibble theStatus, MidiNibble theChannel)
    : UmpEvent ()
    {
        // we need to add the properties for the raw data words to the value tree.
        setattr<uint32_t> (UmpWords::data0Id, 0);
        setattr<uint32_t> (UmpWords::data1Id, 0);
        messageType = MessageTypes::channelVoice2;
        group       = theGroup;
        status      = theStatus;
        channel     = theChannel;
    }

    MAKE_BITFIELD (int, group, 0, 4, 24);
    MAKE_BITFIELD (int, status, 0, 4, 20);
    MAKE_BITFIELD (int, channel, 0, 4, 16);
};

struct Midi2PerNoteEvent : public Midi2ChannelVoiceEvent
{
    Midi2PerNoteEvent (UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
    }
    Midi2PerNoteEvent (MidiNibble theGroup, MidiNibble theStatus, MidiNibble theChannel, MidiByte theNote,
                       uint8_t theController, uint32_t theValue)
    : Midi2ChannelVoiceEvent (theGroup, theStatus, theChannel)
    {
        note       = theNote;
        controller = theController;
        setattr<uint32_t> (UmpWords::data1Id, theValue);
    }

    Midi2PerNoteEvent (MidiNibble theGroup, MidiNibble theStatus, MidiNibble theChannel, MidiByte theNote,
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
};

struct Midi2PerNotePitchBendEvent : public Midi2ChannelVoiceEvent
{
    Midi2PerNotePitchBendEvent (UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
    }

    Midi2PerNotePitchBendEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theNote, int32_t theValue)
    : Midi2ChannelVoiceEvent (theGroup, UmpValues::perNotePitchBend, theChannel)
    {
        note  = theNote;
        value = theValue;
    }

    Midi2PerNotePitchBendEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theNote, MidiBipolarFloat theValue)
    : Midi2PerNotePitchBendEvent (theGroup, theChannel, theNote, theValue.toInt32 ())
    {
    }

    MAKE_BITFIELD (int, note, 0, 8, 8);
    MAKE_BITFIELD (int32_t, value, 1, 32, 0);

    MAKE_COMPUTED_VALUE_MEMBER (
        float, valueFloat, [this] () -> float { return MidiBipolarFloat::fromInt32 (value.get ()); },
        [this] (const MidiBipolarFloat& val) { value = val.toInt32 (); });
};

struct Midi2ControlChangeEvent : public Midi2ChannelVoiceEvent
{
    Midi2ControlChangeEvent (UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
    }

    Midi2ControlChangeEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theController, uint32_t theValue)
    : Midi2ChannelVoiceEvent (theGroup, UmpValues::controlChange, theChannel)
    {
        controller = theController;
        value      = theValue;
    }

    Midi2ControlChangeEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theController,
                             MidiUnipolarFloat theValue)
    : Midi2ControlChangeEvent (theGroup, theChannel, theController, theValue.toUint32 ())
    {
    }

    MAKE_BITFIELD (int, controller, 0, 8, 8);
    MAKE_BITFIELD (uint32_t, value, 1, 32, 0);

    MAKE_COMPUTED_VALUE_MEMBER (
        float, valueFloat, [this] () -> float { return MidiUnipolarFloat::fromUint32 (value.get ()); },
        [this] (const MidiUnipolarFloat& val) { value = val.toUint32 (); });
};

struct Midi2ProgramChangeEvent : public Midi2ChannelVoiceEvent
{
    Midi2ProgramChangeEvent (UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
    }

    Midi2ProgramChangeEvent (MidiNibble theGroup, MidiNibble theChannel, MidiWord theBank, MidiByte theProgram)
    : Midi2ChannelVoiceEvent (theGroup, UmpValues::programChange, theChannel)
    {
        bankValid = true;
        bank      = theBank;
        program   = theProgram;
    }

    Midi2ProgramChangeEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theProgram)
    : Midi2ChannelVoiceEvent (theGroup, UmpValues::programChange, theChannel)
    {
        bankValid = false;
        bankMsb   = 0;
        bankLsb   = 0;
        program   = theProgram;
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
};

struct Midi2PerNoteManagementEvent : public Midi2ChannelVoiceEvent
{
    Midi2PerNoteManagementEvent (UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
    }

    Midi2PerNoteManagementEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theNote, bool theDetach,
                                 bool theReset)
    : Midi2ChannelVoiceEvent (theGroup, UmpValues::perNoteManagement, theChannel)
    {
        jassert (theDetach || theReset);
        note   = theNote;
        detach = theDetach;
        reset  = theReset;
    }

    MAKE_BITFIELD (int, note, 0, 8, 8);
    MAKE_BITFIELD (bool, detach, 0, 1, 1);
    MAKE_BITFIELD (bool, reset, 0, 1, 0);
};

struct Midi2PolyPressureEvent : public Midi2ChannelVoiceEvent
{
    Midi2PolyPressureEvent (UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
    }

    Midi2PolyPressureEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theNote, uint32_t theValue)
    : Midi2ChannelVoiceEvent (theGroup, UmpValues::polyPressure, theChannel)
    {
        note  = theNote;
        value = theValue;
    }

    Midi2PolyPressureEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theNote, MidiUnipolarFloat theValue)
    : Midi2PolyPressureEvent (theGroup, theChannel, theNote, theValue.toUint32 ())
    {
    }

    MAKE_BITFIELD (int, note, 0, 8, 8);
    MAKE_BITFIELD (uint32_t, value, 1, 32, 0);

    MAKE_COMPUTED_VALUE_MEMBER (
        float, valueFloat, [this] () -> float { return MidiUnipolarFloat::fromUint32 (value.get ()); },
        [this] (const MidiUnipolarFloat& val) { value = val.toUint32 (); });
};

struct Midi2ChannelPressureEvent : public Midi2ChannelVoiceEvent
{
    Midi2ChannelPressureEvent (UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
    }

    Midi2ChannelPressureEvent (MidiNibble theGroup, MidiNibble theChannel, uint32_t theValue)
    : Midi2ChannelVoiceEvent (theGroup, UmpValues::channelPressure, theChannel)
    {
        value = theValue;
    }

    Midi2ChannelPressureEvent (MidiNibble theGroup, MidiNibble theChannel, MidiUnipolarFloat theValue)
    : Midi2ChannelPressureEvent (theGroup, theChannel, theValue.toUint32 ())
    {
    }

    MAKE_BITFIELD (uint32_t, value, 1, 32, 0);

    MAKE_COMPUTED_VALUE_MEMBER (
        float, valueFloat, [this] () -> float { return MidiUnipolarFloat::fromUint32 (value.get ()); },
        [this] (const MidiUnipolarFloat& val) { value = val.toUint32 (); });
};

struct Midi2PitchBendEvent : public Midi2ChannelVoiceEvent
{
    Midi2PitchBendEvent (UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
    }

    Midi2PitchBendEvent (MidiNibble theGroup, MidiNibble theChannel, int32_t theValue)
    : Midi2ChannelVoiceEvent (theGroup, UmpValues::pitchBend, theChannel)
    {
        value = theValue;
    }

    Midi2PitchBendEvent (MidiNibble theGroup, MidiNibble theChannel, MidiBipolarFloat theValue)
    : Midi2PitchBendEvent (theGroup, theChannel, theValue.toInt32 ())
    {
    }

    MAKE_BITFIELD (int32_t, value, 1, 32, 0);

    MAKE_COMPUTED_VALUE_MEMBER (
        float, valueFloat, [this] () -> float { return MidiBipolarFloat::fromInt32 (value.get ()); },
        [this] (const MidiBipolarFloat& val) { value = val.toInt32 (); });
};

struct Midi2RegisteredPerNoteControllerEvent : public Midi2PerNoteEvent
{
    Midi2RegisteredPerNoteControllerEvent (UmpEvent& event)
    : Midi2PerNoteEvent (event)
    {
    }
    Midi2RegisteredPerNoteControllerEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theNote,
                                           uint8_t theController, uint32_t theValue)
    : Midi2PerNoteEvent (theGroup, UmpValues::registeredPerNoteController, theChannel, theNote, theController, theValue)
    {
    }

    Midi2RegisteredPerNoteControllerEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theNote,
                                           uint8_t theController, MidiUnipolarFloat theValue)
    : Midi2RegisteredPerNoteControllerEvent (theGroup, theChannel, theNote, theController, theValue.toUint32 ())
    {
    }
};

struct Midi2AssignablePerNoteControllerEvent : public Midi2PerNoteEvent
{
    Midi2AssignablePerNoteControllerEvent (UmpEvent& event)
    : Midi2PerNoteEvent (event)
    {
    }
    Midi2AssignablePerNoteControllerEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theNote,
                                           uint8_t theController, uint32_t theValue)
    : Midi2PerNoteEvent (theGroup, UmpValues::assignablePerNoteController, theChannel, theNote, theController, theValue)
    {
    }

    Midi2AssignablePerNoteControllerEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theNote,
                                           uint8_t theController, MidiUnipolarFloat theValue)
    : Midi2AssignablePerNoteControllerEvent (theGroup, theChannel, theNote, theController, theValue.toUint32 ())
    {
    }
};

struct Midi2ControllerEvent : public Midi2ChannelVoiceEvent
{
    Midi2ControllerEvent (UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
    }
    Midi2ControllerEvent (MidiNibble theGroup, MidiNibble theStatus, MidiNibble theChannel, MidiByte theBank,
                          uint8_t theController, uint32_t theValue)
    : Midi2ChannelVoiceEvent (theGroup, theStatus, theChannel)
    {
        bank       = theBank;
        controller = theController;
        setattr<uint32_t> (UmpWords::data1Id, theValue);
    }

    Midi2ControllerEvent (MidiNibble theGroup, MidiNibble theStatus, MidiNibble theChannel, MidiByte theBank,
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
};

struct Midi2RegisteredControllerEvent : public Midi2ControllerEvent
{
    Midi2RegisteredControllerEvent (UmpEvent& event)
    : Midi2ControllerEvent (event)
    {
    }
    Midi2RegisteredControllerEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theBank, uint8_t theController,
                                    uint32_t theValue)
    : Midi2ControllerEvent (theGroup, UmpValues::registeredController, theChannel, theBank, theController, theValue)
    {
    }

    Midi2RegisteredControllerEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theBank, uint8_t theController,
                                    MidiUnipolarFloat theValue)
    : Midi2RegisteredControllerEvent (theGroup, theChannel, theBank, theController, theValue.toUint32 ())
    {
    }
};

struct Midi2AssignableControllerEvent : public Midi2ControllerEvent
{
    Midi2AssignableControllerEvent (UmpEvent& event)
    : Midi2ControllerEvent (event)
    {
    }
    Midi2AssignableControllerEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theBank, uint8_t theController,
                                    uint32_t theValue)
    : Midi2ControllerEvent (theGroup, UmpValues::assignableController, theChannel, theBank, theController, theValue)
    {
    }

    Midi2AssignableControllerEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theBank, uint8_t theController,
                                    MidiUnipolarFloat theValue)
    : Midi2AssignableControllerEvent (theGroup, theChannel, theBank, theController, theValue.toUint32 ())
    {
    }
};

struct Midi2RelativeControllerEvent : public Midi2ChannelVoiceEvent
{
    Midi2RelativeControllerEvent (UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
    }
    Midi2RelativeControllerEvent (MidiNibble theGroup, MidiNibble theStatus, MidiNibble theChannel, MidiByte theBank,
                                  uint8_t theController, int32_t theValue)
    : Midi2ChannelVoiceEvent (theGroup, theStatus, theChannel)
    {
        bank       = theBank;
        controller = theController;
        setattr<int32_t> (UmpWords::data1Id, theValue);
    }

    Midi2RelativeControllerEvent (MidiNibble theGroup, MidiNibble theStatus, MidiNibble theChannel, MidiByte theBank,
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
};

struct Midi2RelativeRegisteredControllerEvent : public Midi2RelativeControllerEvent
{
    Midi2RelativeRegisteredControllerEvent (UmpEvent& event)
    : Midi2RelativeControllerEvent (event)
    {
    }
    Midi2RelativeRegisteredControllerEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theBank,
                                            uint8_t theController, int32_t theValue)
    : Midi2RelativeControllerEvent (theGroup, UmpValues::relativeRegisteredController, theChannel, theBank,
                                    theController, theValue)
    {
    }

    Midi2RelativeRegisteredControllerEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theBank,
                                            uint8_t theController, MidiBipolarFloat theValue)
    : Midi2RelativeRegisteredControllerEvent (theGroup, theChannel, theBank, theController, theValue.toInt32 ())
    {
    }
};

struct Midi2RelativeAssignableControllerEvent : public Midi2RelativeControllerEvent
{
    Midi2RelativeAssignableControllerEvent (UmpEvent& event)
    : Midi2RelativeControllerEvent (event)
    {
    }
    Midi2RelativeAssignableControllerEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theBank,
                                            uint8_t theController, int32_t theValue)
    : Midi2RelativeControllerEvent (theGroup, UmpValues::relativeAssignableController, theChannel, theBank,
                                    theController, theValue)
    {
    }

    Midi2RelativeAssignableControllerEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theBank,
                                            uint8_t theController, MidiBipolarFloat theValue)
    : Midi2RelativeAssignableControllerEvent (theGroup, theChannel, theBank, theController, theValue.toInt32 ())
    {
    }
};

struct Midi2NoteEvent : public Midi2ChannelVoiceEvent
{
    Midi2NoteEvent (UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
    }

    Midi2NoteEvent (MidiNibble theGroup, MidiNibble theStatus, MidiNibble theChannel, MidiByte theNote,
                    uint16_t theVelocity)
    : Midi2ChannelVoiceEvent (theGroup, theStatus, theChannel)
    {
        note     = theNote;
        velocity = theVelocity;
    }

    Midi2NoteEvent (MidiNibble theGroup, MidiNibble theStatus, MidiNibble theChannel, MidiByte theNote,
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
};

struct Midi2NoteOffEvent : public Midi2NoteEvent
{
    Midi2NoteOffEvent (UmpEvent& event)
    : Midi2NoteEvent (event)
    {
    }

    Midi2NoteOffEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theNote, MidiUnipolarFloat theVelocity)
    : Midi2NoteOffEvent (theGroup, theChannel, theNote, theVelocity.toUint16 ())
    {
    }

    Midi2NoteOffEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theNote, uint16_t theVelocity)
    : Midi2NoteEvent (theGroup, UmpValues::noteOff, theChannel, theNote, theVelocity)
    {
    }
};

struct Midi2NoteOnEvent : public Midi2NoteEvent
{
    Midi2NoteOnEvent (UmpEvent& event)
    : Midi2NoteEvent (event)
    {
    }
    Midi2NoteOnEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theNote, MidiUnipolarFloat theVelocity)
    : Midi2NoteOnEvent (theGroup, theChannel, theNote, theVelocity.toUint16 ())
    {
    }
    Midi2NoteOnEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theNote, uint16_t theVelocity)
    : Midi2NoteEvent (theGroup, UmpValues::noteOn, theChannel, theNote, theVelocity)
    {
    }
};