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
        group   = theGroup;
        status  = theStatus;
        channel = theChannel;
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
                       MidiByte theController, uint32_t theValue)
    : Midi2ChannelVoiceEvent (theGroup, theStatus, theChannel)
    {
        note       = theNote;
        controller = theController;
        setattr<uint32_t> (UmpWords::data1Id, theValue);
    }

    Midi2PerNoteEvent (MidiNibble theGroup, MidiNibble theStatus, MidiNibble theChannel, MidiByte theNote,
                       MidiByte theController, MidiUnipolarFloat theValue)
    : Midi2ChannelVoiceEvent (theGroup, theStatus, theChannel)
    {
        note       = theNote;
        controller = theController;

        setattr<uint32_t> (UmpWords::data1Id,
                           static_cast<uint32_t> (0.5f + theValue * static_cast<float> (UINT32_MAX)));
    }

    MAKE_BITFIELD (int, note, 0, 7, 8);
    MAKE_BITFIELD (int, controller, 0, 7, 0);
};

struct Midi2RegisteredPerNoteControllerEvent : public Midi2PerNoteEvent
{
    Midi2RegisteredPerNoteControllerEvent (UmpEvent& event)
    : Midi2PerNoteEvent (event)
    {
    }
    Midi2RegisteredPerNoteControllerEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theNote,
                                           MidiByte theController, uint32_t theValue)
    : Midi2PerNoteEvent (theGroup, UmpValues::registeredPerNoteController, theChannel, theNote, theController, theValue)
    {
    }

    Midi2RegisteredPerNoteControllerEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theNote,
                                           MidiByte theController, MidiUnipolarFloat theValue)
    : Midi2PerNoteEvent (theGroup, UmpValues::registeredPerNoteController, theChannel, theNote, theController, theValue)
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
                                           MidiByte theController, uint32_t theValue)
    : Midi2PerNoteEvent (theGroup, UmpValues::assignablePerNoteController, theChannel, theNote, theController, theValue)
    {
    }

    Midi2AssignablePerNoteControllerEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theNote,
                                           MidiByte theController, MidiUnipolarFloat theValue)
    : Midi2PerNoteEvent (theGroup, UmpValues::assignablePerNoteController, theChannel, theNote, theController, theValue)
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
    : Midi2ChannelVoiceEvent (theGroup, theStatus, theChannel)
    {
        note     = theNote;
        velocity = static_cast<uint16_t> (0.5f + theVelocity * 65535.0f);
    }

    MAKE_BITFIELD (int, note, 0, 7, 8);
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
    : Midi2NoteEvent (theGroup, UmpValues::noteOff, theChannel, theNote, theVelocity)
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
    : Midi2NoteEvent (theGroup, UmpValues::noteOn, theChannel, theNote, theVelocity)
    {
    }
    Midi2NoteOnEvent (MidiNibble theGroup, MidiNibble theChannel, MidiByte theNote, uint16_t theVelocity)
    : Midi2NoteEvent (theGroup, UmpValues::noteOn, theChannel, theNote, theVelocity)
    {
    }
};