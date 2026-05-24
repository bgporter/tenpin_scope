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

#include "model/ci/ciConstants.h"
#include "model/message.h"
#include "model/midiTypes.h"
#include "utility/buffer.h"

struct Sysex7Message;

// ---------------------------------------------------------------------------
// Shared serialization helpers used by all CI toSysex7Message() implementations.

namespace CiSerial
{

// Append a 28-bit value as 4 × 7-bit bytes, LSB first.
inline void appendMidiLong (Buffer& buf, int value)
{
    MidiLong ml { value };
    buf.append (static_cast<uint8_t> (ml.getLsb ()));
    buf.append (static_cast<uint8_t> (ml.getByte2 ()));
    buf.append (static_cast<uint8_t> (ml.getByte3 ()));
    buf.append (static_cast<uint8_t> (ml.getMsb ()));
}

// Read a 28-bit value from 4 × 7-bit bytes at buf[offset], LSB first.
inline int parseMidiLong (const Buffer& buf, size_t offset)
{
    return MidiLong { static_cast<int> (buf[offset]),
                      static_cast<int> (buf[offset + 1]),
                      static_cast<int> (buf[offset + 2]),
                      static_cast<int> (buf[offset + 3]) }.get ();
}

// Append the 13-byte common CI SysEx header and both MUIDs.
// deviceId defaults to CiDeviceId::functionBlock (0x7F) for PE and most CI messages.
inline void appendCommonHeader (Buffer& buf, int msgType, int msgFmt,
                                int src, int dst,
                                int deviceId = CiDeviceId::functionBlock)
{
    buf.append (0x7E);
    buf.append (static_cast<uint8_t> (deviceId));
    buf.append (static_cast<uint8_t> (CiSubId::midiCi));
    buf.append (static_cast<uint8_t> (msgType));
    buf.append (static_cast<uint8_t> (std::max (msgFmt, messageFormatMin)));
    appendMidiLong (buf, src);
    appendMidiLong (buf, dst);
}

} // namespace CiSerial

// ---------------------------------------------------------------------------

/**
 * @brief Abstract base for all MIDI-CI assembled messages.
 *
 * Carries the common CI header fields shared by every CI message type.
 * Concrete subclasses set their own static type identifier and add
 * message-specific fields.
 *
 * Buffer byte layout (from the assembled Sysex7Message):
 *   [0]     0x7E  universal sysex  (not stored)
 *   [1]     deviceId
 *   [2]     0x0D  MIDI-CI sub-id 1 (not stored)
 *   [3]     messageType
 *   [4]     messageFormat
 *   [5..8]  sourceMuid (4 bytes, LSB first)
 *   [9..12] destMuid   (4 bytes, LSB first)
 *   [13..]  message-specific data
 */
struct CiMessage : public MessageBase
{
    MAKE_VALUE_MEMBER (int, deviceId, 0);
    MAKE_VALUE_MEMBER (int, messageType, 0);
    MAKE_VALUE_MEMBER (int, messageFormat, 0);
    MAKE_VALUE_MEMBER (int, sourceMuid, 0);
    MAKE_VALUE_MEMBER (int, destMuid, 0);
    MAKE_VALUE_MEMBER (Buffer::Ptr, extraData, {});

    bool isBroadcast () const { return destMuid.get () == broadcastMuid; }

    static bool isCiMessage (const Sysex7Message& msg);
    static int  getCiMessageType (const Sysex7Message& msg);

protected:
    explicit CiMessage (const juce::String& typeName);
    CiMessage (const juce::String& typeName, juce::ValueTree vt);

    void parseHeader (const Sysex7Message& msg);

private:
    using MessageBase::channel;
    using MessageBase::channelId;
    using MessageBase::userChannel;
    using MessageBase::userChannelId;
};
