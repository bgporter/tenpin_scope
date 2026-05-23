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

#include "ciMessage.h"

#include "model/sysex/sysex7Message.h"

namespace
{
constexpr size_t kMinCiSize    = 13; // minimum bytes for a valid CI header
constexpr size_t kDeviceIdIdx  = 1;
constexpr size_t kMsgTypeIdx   = 3;
constexpr size_t kMsgFmtIdx    = 4;
constexpr size_t kSrcMuidIdx   = 5;
constexpr size_t kDstMuidIdx   = 9;
constexpr size_t kPayloadStart = 13;
} // namespace

CiMessage::CiMessage (const juce::String& typeName)
: MessageBase { typeName }
{
}

CiMessage::CiMessage (const juce::String& typeName, juce::ValueTree vt)
: MessageBase { typeName, vt }
{
}

bool CiMessage::isCiMessage (const Sysex7Message& msg)
{
    const auto buf = msg.data.get ();
    return buf && buf->size () >= kMinCiSize && (*buf)[0] == 0x7E &&
           (*buf)[2] == static_cast<uint8_t> (CiSubId::midiCi);
}

int CiMessage::getCiMessageType (const Sysex7Message& msg)
{
    const auto buf = msg.data.get ();
    if (!buf || buf->size () <= kMsgTypeIdx)
        return -1;
    return static_cast<int> ((*buf)[kMsgTypeIdx]);
}

void CiMessage::parseHeader (const Sysex7Message& msg)
{
    const auto buf = msg.data.get ();
    if (!buf)
        return;

    group       = msg.group.get ();
    deviceId    = static_cast<int> ((*buf)[kDeviceIdIdx]);
    messageType = static_cast<int> ((*buf)[kMsgTypeIdx]);
    messageFormat = static_cast<int> ((*buf)[kMsgFmtIdx]);

    sourceMuid = MidiLong { static_cast<int> ((*buf)[kSrcMuidIdx]),
                            static_cast<int> ((*buf)[kSrcMuidIdx + 1]),
                            static_cast<int> ((*buf)[kSrcMuidIdx + 2]),
                            static_cast<int> ((*buf)[kSrcMuidIdx + 3]) }
                     .get ();

    destMuid = MidiLong { static_cast<int> ((*buf)[kDstMuidIdx]),
                          static_cast<int> ((*buf)[kDstMuidIdx + 1]),
                          static_cast<int> ((*buf)[kDstMuidIdx + 2]),
                          static_cast<int> ((*buf)[kDstMuidIdx + 3]) }
                   .get ();

    timestamp     = msg.timestamp.get ();
    endpointIndex = msg.endpointIndex.get ();
    endpointName  = msg.endpointName.get ();
    isReceived    = msg.isReceived.get ();
}
