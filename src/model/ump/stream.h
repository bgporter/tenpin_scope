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

namespace StreamStatus
{
constexpr int endpointDiscovery             = 0x00;
constexpr int endpointInfoNotification      = 0x01;
constexpr int deviceIdentityNotification    = 0x02;
constexpr int endpointNameNotification      = 0x03;
constexpr int productInstanceId             = 0x04;
constexpr int streamConfigRequest           = 0x05;
constexpr int streamConfigNotification      = 0x06;
constexpr int functionBlockDiscovery        = 0x10;
constexpr int functionBlockInfoNotification = 0x11;
constexpr int functionBlockNameNotification = 0x12;
constexpr int startOfClip                   = 0x20;
constexpr int endOfClip                     = 0x21;
} // namespace StreamStatus

enum class StreamFormat
{
    complete  = 0,
    start     = 1,
    continue_ = 2,
    end       = 3,
};

namespace juce
{
template <> struct VariantConverter<StreamFormat>
{
    static StreamFormat fromVar (const juce::var& v)
    {
        return static_cast<StreamFormat> (std::clamp (static_cast<int> (v), 0, 3));
    }
    static juce::var toVar (StreamFormat val) { return static_cast<int> (val); }
};
} // namespace juce

// ---------------------------------------------------------------------------

struct StreamEvent : public UmpEvent
{
    StreamEvent (const UmpEvent& event)
    : UmpEvent (event)
    {
    }

    // Word 0
    MAKE_BITFIELD (StreamFormat, format, 0, 2, 26);
    MAKE_BITFIELD (int, status, 0, 10, 16);

protected:
    StreamEvent (StreamFormat theFormat, int theStatus)
    : UmpEvent ()
    {
        setattr<uint32_t> (UmpWords::data0Id, 0);
        setattr<uint32_t> (UmpWords::data1Id, 0);
        setattr<uint32_t> (UmpWords::data2Id, 0);
        setattr<uint32_t> (UmpWords::data3Id, 0);
        messageType = MessageTypes::stream;
        format      = theFormat;
        status      = theStatus;
    }
};

// ---------------------------------------------------------------------------

struct EndpointDiscoveryEvent : public StreamEvent
{
    EndpointDiscoveryEvent (const UmpEvent& event)
    : StreamEvent (event)
    {
        init ();
    }

    EndpointDiscoveryEvent (int theUmpVersionMajor, int theUmpVersionMinor, bool theRequestEndpointInfo,
                            bool theRequestDeviceIdentity, bool theRequestEndpointName,
                            bool theRequestProductInstanceId, bool theRequestStreamConfig)
    : StreamEvent (StreamFormat::complete, StreamStatus::endpointDiscovery)
    {
        init ();
        umpVersionMajor          = theUmpVersionMajor;
        umpVersionMinor          = theUmpVersionMinor;
        requestEndpointInfo      = theRequestEndpointInfo;
        requestDeviceIdentity    = theRequestDeviceIdentity;
        requestEndpointName      = theRequestEndpointName;
        requestProductInstanceId = theRequestProductInstanceId;
        requestStreamConfig      = theRequestStreamConfig;
    }

    // Word 0
    MAKE_BITFIELD (int, umpVersionMajor, 0, 8, 8);
    MAKE_BITFIELD (int, umpVersionMinor, 0, 8, 0);

    // Word 1 — filter bitmap as individual flags
    MAKE_BITFIELD (bool, requestEndpointInfo, 1, 1, 0);
    MAKE_BITFIELD (bool, requestDeviceIdentity, 1, 1, 1);
    MAKE_BITFIELD (bool, requestEndpointName, 1, 1, 2);
    MAKE_BITFIELD (bool, requestProductInstanceId, 1, 1, 3);
    MAKE_BITFIELD (bool, requestStreamConfig, 1, 1, 4);

private:
    void init () { eventName = "Stream: Endpoint Discovery"; }
};

// ---------------------------------------------------------------------------

struct EndpointInfoNotificationEvent : public StreamEvent
{
    EndpointInfoNotificationEvent (const UmpEvent& event)
    : StreamEvent (event)
    {
        init ();
    }

    EndpointInfoNotificationEvent (int theUmpVersionMajor, int theUmpVersionMinor, bool theStaticFunctionBlocks,
                                   int theNumFunctionBlocks, bool theMidi2Protocol, bool theMidi1Protocol,
                                   bool theRxJrTimestamp, bool theTxJrTimestamp)
    : StreamEvent (StreamFormat::complete, StreamStatus::endpointInfoNotification)
    {
        init ();
        umpVersionMajor      = theUmpVersionMajor;
        umpVersionMinor      = theUmpVersionMinor;
        staticFunctionBlocks = theStaticFunctionBlocks;
        numFunctionBlocks    = theNumFunctionBlocks;
        midi2Protocol        = theMidi2Protocol;
        midi1Protocol        = theMidi1Protocol;
        rxJrTimestamp        = theRxJrTimestamp;
        txJrTimestamp        = theTxJrTimestamp;
    }

    // Word 0
    MAKE_BITFIELD (int, umpVersionMajor, 0, 8, 8);
    MAKE_BITFIELD (int, umpVersionMinor, 0, 8, 0);

    // Word 1
    MAKE_BITFIELD (bool, staticFunctionBlocks, 1, 1, 31);
    MAKE_BITFIELD (int, numFunctionBlocks, 1, 7, 24);
    MAKE_BITFIELD (bool, midi2Protocol, 1, 1, 9);
    MAKE_BITFIELD (bool, midi1Protocol, 1, 1, 8);
    MAKE_BITFIELD (bool, rxJrTimestamp, 1, 1, 1);
    MAKE_BITFIELD (bool, txJrTimestamp, 1, 1, 0);

private:
    void init () { eventName = "Stream: Endpoint Info Notification"; }
};

// ---------------------------------------------------------------------------

struct DeviceIdentityNotificationEvent : public StreamEvent
{
    DeviceIdentityNotificationEvent (const UmpEvent& event)
    : StreamEvent (event)
    {
        init ();
    }

    DeviceIdentityNotificationEvent (MidiByte theMfrId1, MidiByte theMfrId2, MidiByte theMfrId3,
                                     MidiWord theDeviceFamily, MidiWord theModelNumber, MidiByte theSwRev1,
                                     MidiByte theSwRev2, MidiByte theSwRev3, MidiByte theSwRev4)
    : StreamEvent (StreamFormat::complete, StreamStatus::deviceIdentityNotification)
    {
        init ();
        mfrId1       = theMfrId1;
        mfrId2       = theMfrId2;
        mfrId3       = theMfrId3;
        deviceFamily = theDeviceFamily;
        modelNumber  = theModelNumber;
        swRev1       = theSwRev1;
        swRev2       = theSwRev2;
        swRev3       = theSwRev3;
        swRev4       = theSwRev4;
    }

    // Word 1 — manufacturer SysEx ID as three 7-bit bytes (bits 31:24 reserved)
    MAKE_BITFIELD (int, mfrId1, 1, 7, 16);
    MAKE_BITFIELD (int, mfrId2, 1, 7, 8);
    MAKE_BITFIELD (int, mfrId3, 1, 7, 0);

    // Word 2 — device family and model number, each a 14-bit value stored as two 7-bit bytes
    MAKE_COMPUTED_VALUE_MEMBER (
        int, deviceFamily, [this] () -> int { return (familyMsb_.get () << 7) | familyLsb_.get (); },
        [this] (const int& val)
        {
            familyLsb_ = val & 0x7F;
            familyMsb_ = (val >> 7) & 0x7F;
        });

    MAKE_COMPUTED_VALUE_MEMBER (
        int, modelNumber, [this] () -> int { return (modelMsb_.get () << 7) | modelLsb_.get (); },
        [this] (const int& val)
        {
            modelLsb_ = val & 0x7F;
            modelMsb_ = (val >> 7) & 0x7F;
        });

    // Word 3 — software revision as four 7-bit bytes
    MAKE_BITFIELD (int, swRev1, 3, 7, 24);
    MAKE_BITFIELD (int, swRev2, 3, 7, 16);
    MAKE_BITFIELD (int, swRev3, 3, 7, 8);
    MAKE_BITFIELD (int, swRev4, 3, 7, 0);

private:
    MAKE_BITFIELD (int, familyLsb_, 2, 7, 24);
    MAKE_BITFIELD (int, familyMsb_, 2, 7, 16);
    MAKE_BITFIELD (int, modelLsb_, 2, 7, 8);
    MAKE_BITFIELD (int, modelMsb_, 2, 7, 0);

    void init () { eventName = "Stream: Device Identity Notification"; }
};

// ---------------------------------------------------------------------------

struct StreamConfigurationRequestEvent : public StreamEvent
{
    StreamConfigurationRequestEvent (const UmpEvent& event)
    : StreamEvent (event)
    {
        init ();
    }

    // protocol: 1 = MIDI 1.0, 2 = MIDI 2.0
    StreamConfigurationRequestEvent (int theProtocol, bool theRxJrTimestamp, bool theTxJrTimestamp)
    : StreamEvent (StreamFormat::complete, StreamStatus::streamConfigRequest)
    {
        init ();
        protocol      = theProtocol;
        rxJrTimestamp = theRxJrTimestamp;
        txJrTimestamp = theTxJrTimestamp;
    }

    // Word 0
    MAKE_BITFIELD (int, protocol, 0, 8, 8);
    MAKE_BITFIELD (bool, rxJrTimestamp, 0, 1, 1);
    MAKE_BITFIELD (bool, txJrTimestamp, 0, 1, 0);

private:
    void init () { eventName = "Stream: Stream Configuration Request"; }
};

// ---------------------------------------------------------------------------

struct StreamConfigurationNotificationEvent : public StreamEvent
{
    StreamConfigurationNotificationEvent (const UmpEvent& event)
    : StreamEvent (event)
    {
        init ();
    }

    // protocol: 1 = MIDI 1.0, 2 = MIDI 2.0
    StreamConfigurationNotificationEvent (int theProtocol, bool theRxJrTimestamp, bool theTxJrTimestamp)
    : StreamEvent (StreamFormat::complete, StreamStatus::streamConfigNotification)
    {
        init ();
        protocol      = theProtocol;
        rxJrTimestamp = theRxJrTimestamp;
        txJrTimestamp = theTxJrTimestamp;
    }

    // Word 0
    MAKE_BITFIELD (int, protocol, 0, 8, 8);
    MAKE_BITFIELD (bool, rxJrTimestamp, 0, 1, 1);
    MAKE_BITFIELD (bool, txJrTimestamp, 0, 1, 0);

private:
    void init () { eventName = "Stream: Stream Configuration Notification"; }
};

// ---------------------------------------------------------------------------

struct FunctionBlockDiscoveryEvent : public StreamEvent
{
    FunctionBlockDiscoveryEvent (const UmpEvent& event)
    : StreamEvent (event)
    {
        init ();
    }

    // functionBlockNumber: 0–30 for a specific block, 0xFF = request all
    FunctionBlockDiscoveryEvent (int theFunctionBlockNumber, bool theRequestInfo, bool theRequestName)
    : StreamEvent (StreamFormat::complete, StreamStatus::functionBlockDiscovery)
    {
        init ();
        functionBlockNumber = theFunctionBlockNumber;
        requestInfo         = theRequestInfo;
        requestName         = theRequestName;
    }

    // Word 0
    MAKE_BITFIELD (int, functionBlockNumber, 0, 8, 8);
    MAKE_BITFIELD (bool, requestInfo, 0, 1, 0);
    MAKE_BITFIELD (bool, requestName, 0, 1, 1);

private:
    void init () { eventName = "Stream: Function Block Discovery"; }
};

// ---------------------------------------------------------------------------

struct FunctionBlockInfoNotificationEvent : public StreamEvent
{
    FunctionBlockInfoNotificationEvent (const UmpEvent& event)
    : StreamEvent (event)
    {
        init ();
    }

    FunctionBlockInfoNotificationEvent (bool theActive, int theFunctionBlockNumber, int theUiHint, int theMidi1,
                                        int theDirection, int theFirstGroup, int theNumGroups, int theNumMidi1Channels,
                                        int theMaxSysex8Streams)
    : StreamEvent (StreamFormat::complete, StreamStatus::functionBlockInfoNotification)
    {
        init ();
        active              = theActive;
        functionBlockNumber = theFunctionBlockNumber;
        uiHint              = theUiHint;
        midi1               = theMidi1;
        direction           = theDirection;
        firstGroup          = theFirstGroup;
        numGroups           = theNumGroups;
        numMidi1Channels    = theNumMidi1Channels;
        maxSysex8Streams    = theMaxSysex8Streams;
    }

    // Word 0
    MAKE_BITFIELD (bool, active, 0, 1, 15);
    MAKE_BITFIELD (int, functionBlockNumber, 0, 7, 8);
    MAKE_BITFIELD (int, uiHint, 0, 2, 6);
    MAKE_BITFIELD (int, midi1, 0, 2, 4);
    MAKE_BITFIELD (int, direction, 0, 2, 2);

    // Word 1
    MAKE_BITFIELD (int, firstGroup, 1, 8, 24);
    MAKE_BITFIELD (int, numGroups, 1, 8, 16);
    MAKE_BITFIELD (int, numMidi1Channels, 1, 8, 8);
    MAKE_BITFIELD (int, maxSysex8Streams, 1, 8, 0);

private:
    void init () { eventName = "Stream: Function Block Info Notification"; }
};

// ---------------------------------------------------------------------------

struct StartOfClipEvent : public StreamEvent
{
    StartOfClipEvent (const UmpEvent& event)
    : StreamEvent (event)
    {
        init ();
    }

    StartOfClipEvent ()
    : StreamEvent (StreamFormat::complete, StreamStatus::startOfClip)
    {
        init ();
    }

private:
    void init () { eventName = "Stream: Start of Clip"; }
};

// ---------------------------------------------------------------------------

struct EndOfClipEvent : public StreamEvent
{
    EndOfClipEvent (const UmpEvent& event)
    : StreamEvent (event)
    {
        init ();
    }

    EndOfClipEvent ()
    : StreamEvent (StreamFormat::complete, StreamStatus::endOfClip)
    {
        init ();
    }

private:
    void init () { eventName = "Stream: End of Clip"; }
};

// ---------------------------------------------------------------------------

class StreamTextEventFactory;

// Covers EndpointNameNotification (0x003), ProductInstanceIdNotification (0x004),
// and FunctionBlockNameNotification (0x012). For the function block variant,
// word 0 bits 15:8 hold the function block number; for the others those bits are
// reserved (zero). Text bytes are packed big-endian across words 1–3 (12 bytes max
// per packet).
struct StreamTextEvent : public StreamEvent
{
    // Deserialization constructor
    StreamTextEvent (const UmpEvent& event)
    : StreamEvent (event)
    {
        init ();
    }

    // Word 0, bits 15:8 — function block number for FunctionBlockNameNotification,
    // reserved (zero) for EndpointName and ProductInstanceId.
    MAKE_BITFIELD (int, functionBlockNumber, 0, 8, 8);

    // Byte accessor: index 0–11 across words 1–3, big-endian within each word.
    uint8_t operator[] (int i) const
    {
        const juce::Identifier& id = (i < 4) ? UmpWords::data1Id : (i < 8) ? UmpWords::data2Id : UmpWords::data3Id;
        const uint32_t word        = getattr<uint32_t> (id, 0);
        return static_cast<uint8_t> ((word >> (24 - (i % 4) * 8)) & 0xFF);
    }

    static constexpr int maxBytes = 12;

private:
    friend class StreamTextEventFactory;

    void init ()
    {
        switch (status.get ())
        {
            case StreamStatus::endpointNameNotification:
                eventName = "Stream: Endpoint Name Notification"; break;
            case StreamStatus::productInstanceId:
                eventName = "Stream: Product Instance Id"; break;
            case StreamStatus::functionBlockNameNotification:
                eventName = "Stream: Function Block Name Notification"; break;
            default: break;
        }
    }

    StreamTextEvent (StreamFormat theFmt, int theStatus, int theFunctionBlockNumber, const uint8_t* data, int count)
    : StreamEvent (theFmt, theStatus)
    {
        init ();
        functionBlockNumber               = theFunctionBlockNumber;
        const juce::Identifier wordIds[3] = { UmpWords::data1Id, UmpWords::data2Id, UmpWords::data3Id };
        for (int w = 0; w < 3; ++w)
        {
            uint32_t word = 0;
            for (int b = 0; b < 4; ++b)
            {
                const int idx = w * 4 + b;
                if (idx < count)
                    word |= static_cast<uint32_t> (data[idx]) << (24 - b * 8);
            }
            setattr<uint32_t> (wordIds[w], word);
        }
    }
};

// ---------------------------------------------------------------------------

class StreamTextEventFactory
{
public:
    using Callback = std::function<void (StreamTextEvent)>;

    StreamTextEventFactory (Callback cb)
    : callback (std::move (cb))
    {
    }

    // theStatus: StreamStatus::endpointNameNotification, productInstanceId, or
    //            functionBlockNameNotification.
    // theFunctionBlockNumber: used only for functionBlockNameNotification; pass 0
    //                         for the other two status values.
    void createEvents (int theStatus, int theFunctionBlockNumber, std::string_view utf8Text)
    {
        const auto* data = reinterpret_cast<const uint8_t*> (utf8Text.data ());
        const int total  = static_cast<int> (utf8Text.size ());

        if (total == 0)
        {
            callback (StreamTextEvent (StreamFormat::complete, theStatus, theFunctionBlockNumber, nullptr, 0));
            return;
        }

        const int numPackets = (total + 11) / 12;
        for (int p = 0; p < numPackets; ++p)
        {
            StreamFormat fmt;
            if (numPackets == 1)
                fmt = StreamFormat::complete;
            else if (p == 0)
                fmt = StreamFormat::start;
            else if (p == numPackets - 1)
                fmt = StreamFormat::end;
            else
                fmt = StreamFormat::continue_;

            const int offset = p * 12;
            const int count  = std::min (12, total - offset);
            callback (StreamTextEvent (fmt, theStatus, theFunctionBlockNumber, data + offset, count));
        }
    }

private:
    Callback callback;
};
