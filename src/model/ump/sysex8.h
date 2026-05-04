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
#include <tuple>

#include "../midiTypes.h"
#include "umpEvent.h"

/**
 * @brief Describes the sender's intent when ending a SysEx8 message early.
 *
 * Used to construct end packets where all data bytes are zero:
 *   valid   (0x1): stream ID is valid; previous data is intact, no further bytes follow.
 *   unknown (0xF): previous data is incomplete or of unknown quality.
 */
enum class Sysex8EndType
{
    valid   = 0x1,
    unknown = 0xF,
};

struct Sysex8Event : public UmpEvent
{
    // Copy / deserialization constructor.
    Sysex8Event (const UmpEvent& event)
    : UmpEvent (event)
    {
        init ();
    }

    // General construction from a buffer span: carries up to 13 data bytes.
    // numBytes = min(bytes.size(), 13) + 1  (the +1 accounts for the stream ID).
    Sysex8Event (MidiGroup theGroup, SysexStatus theStatus, int theStreamId, std::span<const uint8_t> bytes)
    : Sysex8Event (theGroup, theStatus, theStreamId)
    {
        const int n = std::min ((int) bytes.size (), 13);
        for (int i = 0; i < n; ++i)
            setDataByte (i, bytes[static_cast<size_t> (i)]);
        numBytes = n + 1;
    }

    // Special early-termination end packet.
    // numBytes is set to 0x1 (valid) or 0xF (unknown); all data bytes remain zero.
    Sysex8Event (MidiGroup theGroup, int theStreamId, Sysex8EndType endType)
    : Sysex8Event (theGroup, SysexStatus::end, theStreamId)
    {
        numBytes = static_cast<int> (endType);
    }

    // Word 0
    MAKE_BITFIELD (int, group, 0, 4, 24);
    MAKE_BITFIELD (SysexStatus, status, 0, 4, 20);
    MAKE_BITFIELD (int, numBytes, 0, 4, 16);
    MAKE_BITFIELD (int, streamId, 0, 8, 8);
    MAKE_BITFIELD (int, data0, 0, 8, 0);
    // Word 1
    MAKE_BITFIELD (int, data1, 1, 8, 24);
    MAKE_BITFIELD (int, data2, 1, 8, 16);
    MAKE_BITFIELD (int, data3, 1, 8, 8);
    MAKE_BITFIELD (int, data4, 1, 8, 0);
    // Word 2
    MAKE_BITFIELD (int, data5, 2, 8, 24);
    MAKE_BITFIELD (int, data6, 2, 8, 16);
    MAKE_BITFIELD (int, data7, 2, 8, 8);
    MAKE_BITFIELD (int, data8, 2, 8, 0);
    // Word 3
    MAKE_BITFIELD (int, data9, 3, 8, 24);
    MAKE_BITFIELD (int, data10, 3, 8, 16);
    MAKE_BITFIELD (int, data11, 3, 8, 8);
    MAKE_BITFIELD (int, data12, 3, 8, 0);

    MAKE_COMPUTED_VALUE_MEMBER (
        int, userGroup, [this] () -> int { return group.get () + 1; }, [this] (const int& val) { group = val - 1; });

    void setDataByte (int index, int val)
    {
        switch (index)
        {
            case 0:  data0  = val; break;
            case 1:  data1  = val; break;
            case 2:  data2  = val; break;
            case 3:  data3  = val; break;
            case 4:  data4  = val; break;
            case 5:  data5  = val; break;
            case 6:  data6  = val; break;
            case 7:  data7  = val; break;
            case 8:  data8  = val; break;
            case 9:  data9  = val; break;
            case 10: data10 = val; break;
            case 11: data11 = val; break;
            default: jassertfalse; [[fallthrough]];
            case 12: data12 = val; break;
        }
    }

    int operator[] (int index) const
    {
        switch (index)
        {
            case 0:
                return data0.get ();
            case 1:
                return data1.get ();
            case 2:
                return data2.get ();
            case 3:
                return data3.get ();
            case 4:
                return data4.get ();
            case 5:
                return data5.get ();
            case 6:
                return data6.get ();
            case 7:
                return data7.get ();
            case 8:
                return data8.get ();
            case 9:
                return data9.get ();
            case 10:
                return data10.get ();
            case 11:
                return data11.get ();
            default:
                jassertfalse;
                [[fallthrough]];
            case 12:
                return data12.get ();
        }
    }

private:
    // Common setup for fresh construction. All four data words are zeroed so
    // unused data/reserved fields are clean. Public constructors delegate here.
    Sysex8Event (MidiGroup theGroup, SysexStatus theStatus, int theStreamId)
    : UmpEvent ()
    {
        init ();
        setattr<uint32_t> (UmpWords::data0Id, 0);
        setattr<uint32_t> (UmpWords::data1Id, 0);
        setattr<uint32_t> (UmpWords::data2Id, 0);
        setattr<uint32_t> (UmpWords::data3Id, 0);
        messageType = MessageTypes::sysex8;
        userGroup   = theGroup;
        status      = theStatus;
        streamId    = theStreamId;
    }

    void init () { eventName = "SysEx 8"; }
};

using Sysex8EventHandler = std::function<void (Sysex8Event)>;

class Sysex8EventFactory
{
public:
    Sysex8EventFactory (Sysex8EventHandler theHandler)
    : handler { std::move (theHandler) }
    {
    }

    void createEvents (MidiGroup group, int streamId, std::span<const uint8_t> data)
    {
        if (!handler)
            return;

        if (data.size () <= 13)
        {
            handler (Sysex8Event (group, SysexStatus::complete, streamId, data));
            return;
        }

        handler (Sysex8Event (group, SysexStatus::start, streamId, data.first (13)));
        data = data.subspan (13);

        while (data.size () > 13)
        {
            handler (Sysex8Event (group, SysexStatus::continue_, streamId, data.first (13)));
            data = data.subspan (13);
        }

        handler (Sysex8Event (group, SysexStatus::end, streamId, data));
    }

private:
    Sysex8EventHandler handler;
};

// ---------------------------------------------------------------------------

/**
 * @brief Convert a 1 or 3 byte Manufacturer ID into the 16-bit
 * format used by Mixed Data Set messages, see section 7.10 of
 * M2-104, the UMP & MIDI 2 protocol spec.
 */

inline int MfrIdToMdsFormat (MidiByte id1, MidiByte id2 = 0, MidiByte id3 = 0)
{
    if (id1 == 0x00)
        return (static_cast<int> (0x8000 | (id2) << 8) | id3);
    else
        return id1;
}

/**
 * @brief Convert a 16-bit Mixed Data Set Manufacturer ID into the 1 or 3 byte format used by MIDI messages.
 * @param mdsId The 16-bit MDS ID.
 * @return A tuple containing the 1 or 3 byte Manufacturer ID values.
 */
inline std::tuple<MidiByte, MidiByte, MidiByte> MdsFormatToMfrId (int mdsId)
{
    if ((mdsId & 0x8000) != 0)
    {
        // 3-byte ID
        MidiByte id1 { 0x00 };
        MidiByte id2 { (mdsId >> 8) & 0x7F };
        MidiByte id3 { mdsId & 0x7F };
        return std::make_tuple (id1, id2, id3);
    }
    else
    {
        // 1-byte ID
        MidiByte id1 { mdsId & 0x7F };
        return std::make_tuple (id1, MidiByte { 0 }, MidiByte { 0 });
    }
}

struct MixedDataSetHeaderEvent : public UmpEvent
{
    MixedDataSetHeaderEvent (const UmpEvent& event)
    : UmpEvent (event)
    {
        init ();
    }

    MixedDataSetHeaderEvent (MidiGroup theGroup, int theMdsId, int theNumValidBytes, int theNumChunks,
                             int theChunkNumber, int theManufacturerId, int theDeviceId, int theSubId1, int theSubId2)
    : MixedDataSetHeaderEvent (theGroup, theMdsId)
    {
        numValidBytes  = theNumValidBytes;
        numChunks      = theNumChunks;
        chunkNumber    = theChunkNumber;
        manufacturerId = theManufacturerId;
        deviceId       = theDeviceId;
        subId1         = theSubId1;
        subId2         = theSubId2;
    }

    // Word 0
    MAKE_BITFIELD (int, group, 0, 4, 24);
    MAKE_BITFIELD (SysexStatus, status, 0, 4, 20);
    MAKE_BITFIELD (int, mdsId, 0, 4, 16);
    MAKE_BITFIELD (int, numValidBytes, 0, 16, 0);
    // Word 1
    MAKE_BITFIELD (int, numChunks, 1, 16, 16);
    MAKE_BITFIELD (int, chunkNumber, 1, 16, 0);
    // Word 2
    MAKE_BITFIELD (int, manufacturerId, 2, 16, 16);
    MAKE_BITFIELD (int, deviceId, 2, 16, 0);
    // Word 3
    MAKE_BITFIELD (int, subId1, 3, 16, 16);
    MAKE_BITFIELD (int, subId2, 3, 16, 0);

    MAKE_COMPUTED_VALUE_MEMBER (
        int, userGroup, [this] () -> int { return group.get () + 1; }, [this] (const int& val) { group = val - 1; });

private:
    MixedDataSetHeaderEvent (MidiGroup theGroup, int theMdsId)
    : UmpEvent ()
    {
        init ();
        setattr<uint32_t> (UmpWords::data0Id, 0);
        setattr<uint32_t> (UmpWords::data1Id, 0);
        setattr<uint32_t> (UmpWords::data2Id, 0);
        setattr<uint32_t> (UmpWords::data3Id, 0);
        messageType = MessageTypes::sysex8;
        userGroup   = theGroup;
        status      = SysexStatus::mdsHeader;
        mdsId       = theMdsId;
    }

    void init () { eventName = "MDS Header"; }
};

// ---------------------------------------------------------------------------

struct MixedDataSetPayloadEvent : public UmpEvent
{
    MixedDataSetPayloadEvent (const UmpEvent& event)
    : UmpEvent (event)
    {
        init ();
    }

    MixedDataSetPayloadEvent (MidiGroup theGroup, int theMdsId, std::span<const uint8_t> bytes)
    : MixedDataSetPayloadEvent (theGroup, theMdsId)
    {
        const int n = std::min ((int) bytes.size (), 14);
        for (int i = 0; i < n; ++i)
            setDataByte (i, bytes[static_cast<size_t> (i)]);
    }

    // Word 0
    MAKE_BITFIELD (int, group, 0, 4, 24);
    MAKE_BITFIELD (SysexStatus, status, 0, 4, 20);
    MAKE_BITFIELD (int, mdsId, 0, 4, 16);
    MAKE_BITFIELD (int, data0, 0, 8, 8);
    MAKE_BITFIELD (int, data1, 0, 8, 0);
    // Word 1
    MAKE_BITFIELD (int, data2, 1, 8, 24);
    MAKE_BITFIELD (int, data3, 1, 8, 16);
    MAKE_BITFIELD (int, data4, 1, 8, 8);
    MAKE_BITFIELD (int, data5, 1, 8, 0);
    // Word 2
    MAKE_BITFIELD (int, data6, 2, 8, 24);
    MAKE_BITFIELD (int, data7, 2, 8, 16);
    MAKE_BITFIELD (int, data8, 2, 8, 8);
    MAKE_BITFIELD (int, data9, 2, 8, 0);
    // Word 3
    MAKE_BITFIELD (int, data10, 3, 8, 24);
    MAKE_BITFIELD (int, data11, 3, 8, 16);
    MAKE_BITFIELD (int, data12, 3, 8, 8);
    MAKE_BITFIELD (int, data13, 3, 8, 0);

    MAKE_COMPUTED_VALUE_MEMBER (
        int, userGroup, [this] () -> int { return group.get () + 1; }, [this] (const int& val) { group = val - 1; });

    void setDataByte (int index, int val)
    {
        switch (index)
        {
            case 0:  data0  = val; break;
            case 1:  data1  = val; break;
            case 2:  data2  = val; break;
            case 3:  data3  = val; break;
            case 4:  data4  = val; break;
            case 5:  data5  = val; break;
            case 6:  data6  = val; break;
            case 7:  data7  = val; break;
            case 8:  data8  = val; break;
            case 9:  data9  = val; break;
            case 10: data10 = val; break;
            case 11: data11 = val; break;
            case 12: data12 = val; break;
            default: jassertfalse; [[fallthrough]];
            case 13: data13 = val; break;
        }
    }

    int operator[] (int index) const
    {
        switch (index)
        {
            case 0:
                return data0.get ();
            case 1:
                return data1.get ();
            case 2:
                return data2.get ();
            case 3:
                return data3.get ();
            case 4:
                return data4.get ();
            case 5:
                return data5.get ();
            case 6:
                return data6.get ();
            case 7:
                return data7.get ();
            case 8:
                return data8.get ();
            case 9:
                return data9.get ();
            case 10:
                return data10.get ();
            case 11:
                return data11.get ();
            case 12:
                return data12.get ();
            default:
                jassertfalse;
                [[fallthrough]];
            case 13:
                return data13.get ();
        }
    }

private:
    MixedDataSetPayloadEvent (MidiGroup theGroup, int theMdsId)
    : UmpEvent ()
    {
        init ();
        setattr<uint32_t> (UmpWords::data0Id, 0);
        setattr<uint32_t> (UmpWords::data1Id, 0);
        setattr<uint32_t> (UmpWords::data2Id, 0);
        setattr<uint32_t> (UmpWords::data3Id, 0);
        messageType = MessageTypes::sysex8;
        userGroup   = theGroup;
        status      = SysexStatus::mdsPayload;
        mdsId       = theMdsId;
    }

    void init () { eventName = "MDS Payload"; }
};

// ---------------------------------------------------------------------------

using MixedDataSetHeaderHandler  = std::function<void (MixedDataSetHeaderEvent)>;
using MixedDataSetPayloadHandler = std::function<void (MixedDataSetPayloadEvent)>;

class MixedDataSetFactory
{
public:
    MixedDataSetFactory (MixedDataSetHeaderHandler h, MixedDataSetPayloadHandler p)
    : headerHandler { std::move (h) }
    , payloadHandler { std::move (p) }
    {
    }

    void createEvents (MidiGroup group, int mdsId, int manufacturerId, int deviceId, int subId1, int subId2,
                       std::span<const uint8_t> data)
    {
        if (!headerHandler || !payloadHandler)
            return;

        const int numPayloads = ((int) data.size () + 13) / 14;
        headerHandler (MixedDataSetHeaderEvent (group, mdsId, (int) data.size (), numPayloads, 1, manufacturerId,
                                                deviceId, subId1, subId2));
        while (!data.empty ())
        {
            auto chunk = data.first (std::min (data.size (), (size_t) 14));
            payloadHandler (MixedDataSetPayloadEvent (group, mdsId, chunk));
            data = data.subspan (chunk.size ());
        }
    }

private:
    MixedDataSetHeaderHandler headerHandler;
    MixedDataSetPayloadHandler payloadHandler;
};
