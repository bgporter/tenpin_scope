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

#include "umpEvent.h"

struct Midi2ChannelVoiceEvent : public UmpEvent
{
    Midi2ChannelVoiceEvent (UmpEvent& event)
    : UmpEvent (static_cast<juce::ValueTree> (event))
    {
    }

    Midi2ChannelVoiceEvent (int theGroup, int theStatus, int theChannel)
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

struct Midi2NoteEvent : public Midi2ChannelVoiceEvent
{
    Midi2NoteEvent (UmpEvent& event)
    : Midi2ChannelVoiceEvent (event)
    {
    }

    Midi2NoteEvent (int theGroup, int theStatus, int theChannel, int theNote, int theVelocity)
    : Midi2ChannelVoiceEvent (theGroup, theStatus, theChannel)
    {
        note     = theNote;
        velocity = theVelocity;
    }

    Midi2NoteEvent (int theGroup, int theStatus, int theChannel, int theNote, float theVelocity)
    : Midi2ChannelVoiceEvent (theGroup, theStatus, theChannel)
    {
        note          = theNote;
        velocityFloat = theVelocity;
    }

    MAKE_BITFIELD (int, note, 0, 7, 8);
    MAKE_BITFIELD (int, attributeType, 0, 8, 0);
    MAKE_BITFIELD (int, velocity, 1, 16, 16);
    MAKE_COMPUTED_VALUE_MEMBER (
        float, velocityFloat, [this] () -> float { return velocity.get () / 65535.0f; },
        [this] (const float& val) { velocity = static_cast<int> (0.5f + val * 65535.0f); });
    MAKE_BITFIELD (int, attributeValue, 1, 16, 0);
};

struct Midi2NoteOffEvent : public Midi2NoteEvent
{
    Midi2NoteOffEvent (UmpEvent& event)
    : Midi2NoteEvent (event)
    {
    }
    Midi2NoteOffEvent (int theGroup, int theChannel, int theNote, float theVelocity)
    : Midi2NoteEvent (theGroup, UmpValues::noteOff, theChannel, theNote, theVelocity)
    {
    }
    Midi2NoteOffEvent (int theGroup, int theChannel, int theNote, int theVelocity)
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
    Midi2NoteOnEvent (int theGroup, int theChannel, int theNote, float theVelocity)
    : Midi2NoteEvent (theGroup, UmpValues::noteOn, theChannel, theNote, theVelocity)
    {
    }
    Midi2NoteOnEvent (int theGroup, int theChannel, int theNote, int theVelocity)
    : Midi2NoteEvent (theGroup, UmpValues::noteOn, theChannel, theNote, theVelocity)
    {
    }
};