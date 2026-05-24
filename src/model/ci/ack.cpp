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

#include "ack.h"

namespace
{
constexpr size_t kOrigSubIdIdx = 13;
constexpr size_t kStatusCode   = 14;
constexpr size_t kStatusData   = 15;
constexpr size_t kDetailStart  = 16; // 5 bytes: [16..20]
constexpr size_t kMlLsb        = 21; // 2-byte message length LSB first
constexpr size_t kTextStart    = 23;
constexpr size_t kMinSize      = 23; // header(13) + 8 fixed payload + 2 ml bytes
} // namespace

using CiSerial::appendMidiLong;

namespace {

// Accept 7-bit ASCII printable chars plus line feed; skip anything else.
juce::String parseMessageText (const Buffer& buf, size_t start, size_t count)
{
    juce::String s;
    for (size_t i = 0; i < count && (start + i) < buf.size (); ++i)
    {
        const uint8_t c = buf[start + i];
        if (c == 0x0A || (c >= 0x20 && c <= 0x7E))
            s += static_cast<char> (c);
    }
    return s;
}
} // namespace

// ============================================================================
// CiNak
// ============================================================================

CiNak::CiNak (const Sysex7Message& msg)
: CiMessage { type.toString () }
{
    parseHeader (msg);
    const auto buf = msg.data.get ();
    if (!buf || buf->size () < kMinSize)
        return;

    originalSubId = static_cast<int> ((*buf)[kOrigSubIdIdx]);
    statusCode    = static_cast<int> ((*buf)[kStatusCode]);
    statusData    = static_cast<int> ((*buf)[kStatusData]);
    detail0    = static_cast<int> ((*buf)[kDetailStart]);
    detail1    = static_cast<int> ((*buf)[kDetailStart + 1]);
    detail2    = static_cast<int> ((*buf)[kDetailStart + 2]);
    detail3    = static_cast<int> ((*buf)[kDetailStart + 3]);
    detail4    = static_cast<int> ((*buf)[kDetailStart + 4]);

    const size_t ml = static_cast<size_t> ((*buf)[kMlLsb]) |
                      (static_cast<size_t> ((*buf)[kMlLsb + 1]) << 7);
    messageText = parseMessageText (*buf, kTextStart, ml);
}

CiNak::CiNak (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiNak::CiNak (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiNak::CiNak (MidiGroup theGroup, int sourceMuidValue, int destMuidValue,
              MidiByte origSubId, MidiByte theStatusCode, MidiByte theStatusData,
              const std::array<uint8_t, kDetailBytes>& details,
              const juce::String& text)
: CiMessage { type.toString () }
{
    group         = theGroup.get () - 1;
    deviceId      = CiDeviceId::functionBlock;
    messageType   = CiType::nak;
    messageFormat = messageFormatMin;
    sourceMuid    = sourceMuidValue;
    destMuid      = destMuidValue;

    originalSubId = origSubId.get ();
    statusCode    = theStatusCode.get ();
    statusData    = theStatusData.get ();
    detail0    = details[0];
    detail1    = details[1];
    detail2    = details[2];
    detail3    = details[3];
    detail4    = details[4];
    messageText   = text;
}

Sysex7Message CiNak::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    buf->append (0x7E);
    buf->append (static_cast<uint8_t> (deviceId.get ()));
    buf->append (static_cast<uint8_t> (CiSubId::midiCi));
    buf->append (static_cast<uint8_t> (CiType::nak));
    buf->append (static_cast<uint8_t> (std::max (targetFormat, messageFormatMin)));
    appendMidiLong (*buf, sourceMuid.get ());
    appendMidiLong (*buf, destMuid.get ());

    buf->append (static_cast<uint8_t> (originalSubId.get ()));
    buf->append (static_cast<uint8_t> (statusCode.get ()));
    buf->append (static_cast<uint8_t> (statusData.get ()));
    buf->append (static_cast<uint8_t> (detail0.get ()));
    buf->append (static_cast<uint8_t> (detail1.get ()));
    buf->append (static_cast<uint8_t> (detail2.get ()));
    buf->append (static_cast<uint8_t> (detail3.get ()));
    buf->append (static_cast<uint8_t> (detail4.get ()));

    const auto text = messageText.get ();
    const auto ml   = static_cast<size_t> (text.length ());
    buf->append (static_cast<uint8_t> (ml & 0x7F));
    buf->append (static_cast<uint8_t> ((ml >> 7) & 0x7F));
    for (int i = 0; i < text.length (); ++i)
        buf->append (static_cast<uint8_t> (text[i]));

    return Sysex7Message { theGroup, buf };
}

// ============================================================================
// CiAck
// ============================================================================

CiAck::CiAck (const Sysex7Message& msg)
: CiMessage { type.toString () }
{
    parseHeader (msg);
    const auto buf = msg.data.get ();
    if (!buf || buf->size () < kMinSize)
        return;

    originalSubId = static_cast<int> ((*buf)[kOrigSubIdIdx]);
    statusCode    = static_cast<int> ((*buf)[kStatusCode]);
    statusData    = static_cast<int> ((*buf)[kStatusData]);
    detail0    = static_cast<int> ((*buf)[kDetailStart]);
    detail1    = static_cast<int> ((*buf)[kDetailStart + 1]);
    detail2    = static_cast<int> ((*buf)[kDetailStart + 2]);
    detail3    = static_cast<int> ((*buf)[kDetailStart + 3]);
    detail4    = static_cast<int> ((*buf)[kDetailStart + 4]);

    const size_t ml        = static_cast<size_t> ((*buf)[kMlLsb]) |
                             (static_cast<size_t> ((*buf)[kMlLsb + 1]) << 7);
    messageText = parseMessageText (*buf, kTextStart, ml);
}

CiAck::CiAck (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiAck::CiAck (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiAck::CiAck (MidiGroup theGroup, int sourceMuidValue, int destMuidValue,
              MidiByte origSubId, MidiByte theStatusCode, MidiByte theStatusData,
              const std::array<uint8_t, kDetailBytes>& details,
              const juce::String& text)
: CiMessage { type.toString () }
{
    group         = theGroup.get () - 1;
    deviceId      = CiDeviceId::functionBlock;
    messageType   = CiType::ack;
    messageFormat = messageFormatMin;
    sourceMuid    = sourceMuidValue;
    destMuid      = destMuidValue;

    originalSubId = origSubId.get ();
    statusCode    = theStatusCode.get ();
    statusData    = theStatusData.get ();
    detail0    = details[0];
    detail1    = details[1];
    detail2    = details[2];
    detail3    = details[3];
    detail4    = details[4];
    messageText   = text;
}

Sysex7Message CiAck::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    buf->append (0x7E);
    buf->append (static_cast<uint8_t> (deviceId.get ()));
    buf->append (static_cast<uint8_t> (CiSubId::midiCi));
    buf->append (static_cast<uint8_t> (CiType::ack));
    buf->append (static_cast<uint8_t> (std::max (targetFormat, messageFormatMin)));
    appendMidiLong (*buf, sourceMuid.get ());
    appendMidiLong (*buf, destMuid.get ());

    buf->append (static_cast<uint8_t> (originalSubId.get ()));
    buf->append (static_cast<uint8_t> (statusCode.get ()));
    buf->append (static_cast<uint8_t> (statusData.get ()));
    buf->append (static_cast<uint8_t> (detail0.get ()));
    buf->append (static_cast<uint8_t> (detail1.get ()));
    buf->append (static_cast<uint8_t> (detail2.get ()));
    buf->append (static_cast<uint8_t> (detail3.get ()));
    buf->append (static_cast<uint8_t> (detail4.get ()));

    const auto text = messageText.get ();
    const auto ml   = static_cast<size_t> (text.length ());
    buf->append (static_cast<uint8_t> (ml & 0x7F));
    buf->append (static_cast<uint8_t> ((ml >> 7) & 0x7F));
    for (int i = 0; i < text.length (); ++i)
        buf->append (static_cast<uint8_t> (text[i]));

    return Sysex7Message { theGroup, buf };
}

#if RUN_UNIT_TESTS
#include "test/test_CiAck.inl"
#endif
