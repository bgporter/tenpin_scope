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

#include "endpointInfo.h"

#include "utility/logger.h"

namespace
{
constexpr size_t kStatusIdx   = 13;
constexpr size_t kLidLsb      = 14; // reply only: 2-byte length LSB first
constexpr size_t kDataStart   = 16; // reply only: information data begins here

constexpr size_t kInquiryMinSize = 14; // header (13) + status (1)
constexpr size_t kReplyMinSize   = 16; // header (13) + status (1) + lid (2)

// Validates and filters a product instance ID string. Returns a string
// containing only characters in [32, 126], truncated to maxLen bytes.
juce::String sanitizeProductId (const juce::String& raw, int maxLen)
{
    juce::String result;
    for (auto c : raw)
    {
        if (c < 32 || c > 126)
        {
            WARN_ ({ { "msg", "CiEndpointReply: invalid character in product instance ID" },
                     { "char", static_cast<int> (c) } });
            continue;
        }
        if (result.length () >= maxLen)
        {
            WARN_ ({ { "msg", "CiEndpointReply: product instance ID truncated to max length" },
                     { "maxLen", maxLen } });
            break;
        }
        result += c;
    }
    return result;
}
} // namespace

// ============================================================================
// CiEndpointInquiry
// ============================================================================

CiEndpointInquiry::CiEndpointInquiry (const Sysex7Message& msg)
: CiMessage { type.toString () }
{
    parseHeader (msg);
    const auto buf = msg.data.get ();
    if (!buf || buf->size () < kInquiryMinSize)
        return;
    status = static_cast<int> ((*buf)[kStatusIdx]);
}

CiEndpointInquiry::CiEndpointInquiry (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiEndpointInquiry::CiEndpointInquiry (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiEndpointInquiry::CiEndpointInquiry (MidiGroup theGroup, int sourceMuidValue, int destMuidValue,
                                      MidiByte theStatus)
: CiMessage { type.toString () }
{
    group       = theGroup.get () - 1;
    deviceId    = CiDeviceId::functionBlock;
    messageType = CiType::endpointInquiry;
    messageFormat = messageFormatMin;
    sourceMuid  = sourceMuidValue;
    destMuid    = destMuidValue;
    status      = theStatus.get ();
}

Sysex7Message CiEndpointInquiry::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    buf->append (0x7E);
    buf->append (static_cast<uint8_t> (deviceId.get ()));
    buf->append (static_cast<uint8_t> (CiSubId::midiCi));
    buf->append (static_cast<uint8_t> (CiType::endpointInquiry));
    buf->append (static_cast<uint8_t> (std::max (targetFormat, messageFormatMin)));
    MidiLong srcMl { sourceMuid.get () };
    buf->append (static_cast<uint8_t> (srcMl.getLsb ()));
    buf->append (static_cast<uint8_t> (srcMl.getByte2 ()));
    buf->append (static_cast<uint8_t> (srcMl.getByte3 ()));
    buf->append (static_cast<uint8_t> (srcMl.getMsb ()));
    MidiLong dstMl { destMuid.get () };
    buf->append (static_cast<uint8_t> (dstMl.getLsb ()));
    buf->append (static_cast<uint8_t> (dstMl.getByte2 ()));
    buf->append (static_cast<uint8_t> (dstMl.getByte3 ()));
    buf->append (static_cast<uint8_t> (dstMl.getMsb ()));
    buf->append (static_cast<uint8_t> (status.get ()));
    return Sysex7Message { theGroup, buf };
}

// ============================================================================
// CiEndpointReply
// ============================================================================

CiEndpointReply::CiEndpointReply (const Sysex7Message& msg)
: CiMessage { type.toString () }
{
    parseHeader (msg);
    const auto buf = msg.data.get ();
    if (!buf || buf->size () < kReplyMinSize)
        return;

    status = static_cast<int> ((*buf)[kStatusIdx]);

    const size_t lid = static_cast<size_t> ((*buf)[kLidLsb]) |
                       (static_cast<size_t> ((*buf)[kLidLsb + 1]) << 7);
    const size_t available = (buf->size () > kDataStart) ? buf->size () - kDataStart : 0;
    const size_t toRead    = std::min (lid, available);

    juce::String s;
    for (size_t i = 0; i < toRead; ++i)
    {
        const uint8_t c = (*buf)[kDataStart + i];
        if (c >= 32 && c <= 126)
            s += static_cast<char> (c);
    }
    productInstanceId = s;
}

CiEndpointReply::CiEndpointReply (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiEndpointReply::CiEndpointReply (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiEndpointReply::CiEndpointReply (MidiGroup theGroup, int sourceMuidValue, int destMuidValue,
                                  MidiByte theStatus, const juce::String& productId)
: CiMessage { type.toString () }
{
    group         = theGroup.get () - 1;
    deviceId      = CiDeviceId::functionBlock;
    messageType   = CiType::endpointReply;
    messageFormat = messageFormatMin;
    sourceMuid    = sourceMuidValue;
    destMuid      = destMuidValue;
    status        = theStatus.get ();
    productInstanceId = sanitizeProductId (productId, maxProductInstanceIdLength);
}

Sysex7Message CiEndpointReply::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    buf->append (0x7E);
    buf->append (static_cast<uint8_t> (deviceId.get ()));
    buf->append (static_cast<uint8_t> (CiSubId::midiCi));
    buf->append (static_cast<uint8_t> (CiType::endpointReply));
    buf->append (static_cast<uint8_t> (std::max (targetFormat, messageFormatMin)));
    MidiLong srcMl { sourceMuid.get () };
    buf->append (static_cast<uint8_t> (srcMl.getLsb ()));
    buf->append (static_cast<uint8_t> (srcMl.getByte2 ()));
    buf->append (static_cast<uint8_t> (srcMl.getByte3 ()));
    buf->append (static_cast<uint8_t> (srcMl.getMsb ()));
    MidiLong dstMl { destMuid.get () };
    buf->append (static_cast<uint8_t> (dstMl.getLsb ()));
    buf->append (static_cast<uint8_t> (dstMl.getByte2 ()));
    buf->append (static_cast<uint8_t> (dstMl.getByte3 ()));
    buf->append (static_cast<uint8_t> (dstMl.getMsb ()));
    buf->append (static_cast<uint8_t> (status.get ()));

    const auto id  = productInstanceId.get ();
    const auto len = static_cast<size_t> (id.length ());
    buf->append (static_cast<uint8_t> (len & 0x7F));
    buf->append (static_cast<uint8_t> ((len >> 7) & 0x7F));
    for (int i = 0; i < id.length (); ++i)
        buf->append (static_cast<uint8_t> (id[i]));

    return Sysex7Message { theGroup, buf };
}

#if RUN_UNIT_TESTS
#include "invalidateMuid.h"
#include "test/test_CiEndpointInfo.inl"
#endif
