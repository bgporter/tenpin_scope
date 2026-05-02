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

#include "bitField.h"
#include "utility/variantConverters.h"

namespace MessageTypes
{
constexpr int utility           = 0x0;
constexpr int systemCommon      = 0x1;
constexpr int midi1ChannelVoice = 0x2;
constexpr int sysex7            = 0x3;
constexpr int midi2ChannelVoice = 0x4;
constexpr int sysex8            = 0x5;
constexpr int reserved_6        = 0x6;
constexpr int reserved_7        = 0x7;
constexpr int reserved_8        = 0x8;
constexpr int reserved_9        = 0x9;
constexpr int reserved_a        = 0xa;
constexpr int reserved_b        = 0xb;
constexpr int reserved_c        = 0xc;
constexpr int flexData          = 0xd;
constexpr int reserved_e        = 0xe;
constexpr int stream            = 0xf;
} // namespace MessageTypes

namespace UmpValues
{

namespace Utility
{
constexpr int noop                          = 0x0;
constexpr int jrClock                       = 0x1;
constexpr int jrTimestamp                   = 0x2;
constexpr int deltaClockstampTPQ            = 0x3;
constexpr int deltaClockstampSinceLastEvent = 0x4;
} // namespace Utility

namespace ChannelVoice
{
/// @brief Channel voice status values
constexpr int registeredPerNoteController  = 0x0;
constexpr int assignablePerNoteController  = 0x1;
constexpr int registeredController         = 0x2;
constexpr int assignableController         = 0x3;
constexpr int relativeRegisteredController = 0x4;
constexpr int relativeAssignableController = 0x5;
constexpr int perNotePitchBend             = 0x6;
constexpr int noteOff                      = 0x8;
constexpr int noteOn                       = 0x9;
constexpr int polyPressure                 = 0xA;
constexpr int controlChange                = 0xB;
constexpr int programChange                = 0xC;
constexpr int channelPressure              = 0xD;
constexpr int pitchBend                    = 0xE;
constexpr int perNoteManagement            = 0xF;
} // namespace ChannelVoice

/// @brief  @brief UMP common realtime message status values
namespace SystemCommon
{
// constexpr int reserved            = 0xF0;
// constexpr int reserved2           = 0xF4;
// constexpr int reserved3           = 0xF5;
// constexpr int reserved4           = 0xF7;
// constexpr int reserved5           = 0xF9;

constexpr int midiTimeCode        = 0xF1;
constexpr int songPositionPointer = 0xF2;
constexpr int songSelect          = 0xF3;
constexpr int tuneRequest         = 0xF6;
constexpr int timingClock         = 0xF8;
constexpr int start               = 0xFA;
constexpr int continue_           = 0xFB;
constexpr int stop                = 0xFC;
constexpr int activeSensing       = 0xFE;
constexpr int systemReset         = 0xFF;
} // namespace SystemCommon
} // namespace UmpValues

enum class MtcMessageType
{
    frameCountLsb    = 0,
    frameCountMsb    = 1,
    secondsCountLsb  = 2,
    secondsCountMsb  = 3,
    minutesCountLsb  = 4,
    minutesCountMsb  = 5,
    hoursCountLsb    = 6,
    hoursAndSmpteType = 7,
};

enum class SysexStatus
{
    complete  = 0,
    start     = 1,
    continue_ = 2,
    end       = 3,
};

namespace juce
{
template <> struct VariantConverter<SysexStatus>
{
    static SysexStatus fromVar (const juce::var& v)
    {
        return static_cast<SysexStatus> (std::clamp (static_cast<int> (v), 0, 3));
    }
    static juce::var toVar (SysexStatus val) { return static_cast<int> (val); }
};

template <> struct VariantConverter<MtcMessageType>
{
    static MtcMessageType fromVar (const juce::var& v)
    {
        return static_cast<MtcMessageType> (std::clamp (static_cast<int> (v), 0, 7));
    }
    static juce::var toVar (MtcMessageType val) { return static_cast<int> (val); }
};
} // namespace juce

struct UmpEvent : public cello::Object
{
    static const inline juce::Identifier type { "UmpEvent" };
    UmpEvent (const juce::ump::View& view, double timestamp, int endpointIndex)
    : cello::Object { type.toString (), nullptr }
    {
        // is this too clever?
        switch (view.size ())
        {
            // we use the 'Pythonic' setattr style to create and populate 1 to 4
            // data words at construction time.
            case 4:
                setattr<uint32_t> (UmpWords::data3Id, view[3]);
                [[fallthrough]];
            case 3:
                setattr<uint32_t> (UmpWords::data2Id, view[2]);
                [[fallthrough]];
            case 2:
                setattr<uint32_t> (UmpWords::data1Id, view[1]);
                [[fallthrough]];
            default:
                setattr<uint32_t> (UmpWords::data0Id, view[0]);
                break;
        }
        this->timestamp     = timestamp;
        this->endpointIndex = endpointIndex;
    }

    UmpEvent (const uint32_t* data, size_t size, double timestamp, int endpointIndex)
    : cello::Object { type.toString (), nullptr }
    {
        // Constructor taking raw packet data directly (for thread-safe queueing)
        switch (size)
        {
            case 4:
                setattr<uint32_t> (UmpWords::data3Id, data[3]);
                [[fallthrough]];
            case 3:
                setattr<uint32_t> (UmpWords::data2Id, data[2]);
                [[fallthrough]];
            case 2:
                setattr<uint32_t> (UmpWords::data1Id, data[1]);
                [[fallthrough]];
            default:
                setattr<uint32_t> (UmpWords::data0Id, data[0]);
                break;
        }
        this->timestamp     = timestamp;
        this->endpointIndex = endpointIndex;
    }

    UmpEvent ()
    : cello::Object { type.toString (), nullptr }
    {
    }

    UmpEvent (juce::ValueTree valueTree)
    : cello::Object { type.toString (), valueTree }
    {
        if (valueTree.getType () != type)
        {
            DBG ("Expected " << type.toString () << " got " << valueTree.getType ().toString ());
            jassertfalse;
        }
    }

    MAKE_VALUE_MEMBER (double, timestamp, 0.0);
    MAKE_VALUE_MEMBER (int, endpointIndex, 0);
    MAKE_VALUE_MEMBER (juce::String, endpointName, "");
    MAKE_VALUE_MEMBER (bool, isReceived, false);
    MAKE_VALUE_MEMBER (juce::String, eventName, "UMP Event");

    MAKE_BITFIELD (int, messageType, 0, 4, 28);

    // we'll also create and populate 1 to 4 data words at construction time.
};
