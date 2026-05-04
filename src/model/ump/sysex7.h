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
#include <functional>
#include <span>

#include "../midiTypes.h"
#include "umpEvent.h"

struct Sysex7Event : public UmpEvent
{
    Sysex7Event (const UmpEvent& event)
    : UmpEvent (event)
    {
        init ();
    }

    // Span constructor: slice up to 6 bytes from a raw buffer.
    // numBytes is derived from span size, clamped to 0-6.
    Sysex7Event (MidiGroup theGroup, SysexStatus theStatus, std::span<const uint8_t> bytes)
    : Sysex7Event (theGroup, theStatus)
    {
        const int n = std::min ((int) bytes.size (), 6);
        for (int i = 0; i < n; ++i)
            setDataByte (i, bytes[static_cast<size_t> (i)] & 0x7F);
        numBytes = n;
    }

    // Individual-byte constructor: explicit count + up to 6 MidiByte values.
    // MidiByte validates 0-127 at the call site; no clamping needed in the body.
    Sysex7Event (MidiGroup theGroup, SysexStatus theStatus,
                 int theNumBytes,
                 MidiByte b0 = 0, MidiByte b1 = 0, MidiByte b2 = 0,
                 MidiByte b3 = 0, MidiByte b4 = 0, MidiByte b5 = 0)
    : Sysex7Event (theGroup, theStatus)
    {
        numBytes = std::clamp (theNumBytes, 0, 6);
        data0 = b0; data1 = b1; data2 = b2;
        data3 = b3; data4 = b4; data5 = b5;
    }

    // Word 0
    MAKE_BITFIELD (int,         group,    0, 4, 24);
    MAKE_BITFIELD (SysexStatus, status,   0, 4, 20);
    MAKE_BITFIELD (int,         numBytes, 0, 4, 16);
    MAKE_BITFIELD (int,         data0,    0, 7,  8);
    MAKE_BITFIELD (int,         data1,    0, 7,  0);
    // Word 1
    MAKE_BITFIELD (int,         data2,    1, 7, 24);
    MAKE_BITFIELD (int,         data3,    1, 7, 16);
    MAKE_BITFIELD (int,         data4,    1, 7,  8);
    MAKE_BITFIELD (int,         data5,    1, 7,  0);

    MAKE_COMPUTED_VALUE_MEMBER (
        int, userGroup,
        [this] () -> int { return group.get () + 1; },
        [this] (const int& val) { group = val - 1; });

    void setDataByte (int index, int val)
    {
        switch (index)
        {
            case 0:  data0 = val; break;
            case 1:  data1 = val; break;
            case 2:  data2 = val; break;
            case 3:  data3 = val; break;
            case 4:  data4 = val; break;
            default: jassertfalse; [[fallthrough]];
            case 5:  data5 = val; break;
        }
    }

    int operator[] (int index) const
    {
        switch (index)
        {
            case 0:  return data0.get ();
            case 1:  return data1.get ();
            case 2:  return data2.get ();
            case 3:  return data3.get ();
            case 4:  return data4.get ();
            default: jassertfalse; [[fallthrough]];
            case 5:  return data5.get ();
        }
    }

private:
    // Common setup for fresh construction: zeroes both data words and sets fixed fields.
    // Public constructors delegate here via : Sysex7Event(theGroup, theStatus).
    Sysex7Event (MidiGroup theGroup, SysexStatus theStatus)
    : UmpEvent ()
    {
        init ();
        setattr<uint32_t> (UmpWords::data0Id, 0);
        setattr<uint32_t> (UmpWords::data1Id, 0);
        messageType = MessageTypes::sysex7;
        userGroup   = theGroup;
        status      = theStatus;
    }

    void init () { eventName = "SysEx 7"; }
};

using Sysex7EventHandler = std::function<void(Sysex7Event)>;

class Sysex7EventFactory
{
public:
    Sysex7EventFactory (Sysex7EventHandler theHandler)
    : handler { std::move (theHandler) }
    {}

    void createEvents (MidiGroup group, std::span<const uint8_t> data)
    {
        if (!handler)
            return;

        if (data.size () <= 6)
        {
            handler (Sysex7Event (group, SysexStatus::complete, data));
            return;
        }

        handler (Sysex7Event (group, SysexStatus::start, data.first (6)));
        data = data.subspan (6);

        while (data.size () > 6)
        {
            handler (Sysex7Event (group, SysexStatus::continue_, data.first (6)));
            data = data.subspan (6);
        }

        handler (Sysex7Event (group, SysexStatus::end, data));
    }

private:
    Sysex7EventHandler handler;
};
