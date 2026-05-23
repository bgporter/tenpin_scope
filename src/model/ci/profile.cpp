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

#include "profile.h"

namespace
{
constexpr size_t kCepIdx   = 13; // offset of enabled-profile count (2 bytes LSB first)
constexpr size_t kCountLen = 2;  // byte width of cep / cdp fields
constexpr size_t kMinSize  = 13; // header only — profile lists may be empty

void appendMidiLong (Buffer& buf, int value)
{
    MidiLong ml { value };
    buf.append (static_cast<uint8_t> (ml.getLsb ()));
    buf.append (static_cast<uint8_t> (ml.getByte2 ()));
    buf.append (static_cast<uint8_t> (ml.getByte3 ()));
    buf.append (static_cast<uint8_t> (ml.getMsb ()));
}

void appendCommonHeader (Buffer& buf, int devId, int msgType, int msgFmt, int src, int dst)
{
    buf.append (0x7E);
    buf.append (static_cast<uint8_t> (devId));
    buf.append (static_cast<uint8_t> (CiSubId::midiCi));
    buf.append (static_cast<uint8_t> (msgType));
    buf.append (static_cast<uint8_t> (std::max (msgFmt, messageFormatMin)));
    appendMidiLong (buf, src);
    appendMidiLong (buf, dst);
}

void appendProfileId (Buffer& buf, ProfileId p)
{
    buf.append (p.byte1);
    buf.append (p.byte2);
    buf.append (p.byte3);
    buf.append (p.byte4);
    buf.append (p.byte5);
}

ProfileId profileIdAt (const Buffer& buf, size_t offset)
{
    return { buf[offset], buf[offset + 1], buf[offset + 2], buf[offset + 3], buf[offset + 4] };
}

// Parse a run of profile IDs from buf starting at offset; count is the number of profiles.
Buffer::Ptr parseProfileList (const Buffer& buf, size_t offset, size_t count)
{
    if (count == 0)
        return {};
    Buffer::Ptr out = new Buffer ();
    const size_t end = offset + count * CiProfileInquiryReply::kProfileSize;
    for (size_t i = offset; i < end && i < buf.size (); ++i)
        out->append (buf[i]);
    return out;
}

void ensureBuffer (Buffer::Ptr& buf)
{
    if (!buf)
        buf = new Buffer ();
}
} // namespace

// ============================================================================
// CiProfileInquiry
// ============================================================================

CiProfileInquiry::CiProfileInquiry (const Sysex7Message& msg)
: CiMessage { type.toString () }
{
    parseHeader (msg);
}

CiProfileInquiry::CiProfileInquiry (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiProfileInquiry::CiProfileInquiry (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiProfileInquiry::CiProfileInquiry (MidiGroup theGroup, int sourceMuidValue, int destMuidValue)
: CiMessage { type.toString () }
{
    group         = theGroup.get () - 1;
    deviceId      = CiDeviceId::functionBlock;
    messageType   = CiType::profileInquiry;
    messageFormat = messageFormatMin;
    sourceMuid    = sourceMuidValue;
    destMuid      = destMuidValue;
}

Sysex7Message CiProfileInquiry::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, deviceId.get (), CiType::profileInquiry, targetFormat,
                        sourceMuid.get (), destMuid.get ());
    return Sysex7Message { theGroup, buf };
}

// ============================================================================
// CiProfileInquiryReply
// ============================================================================

CiProfileInquiryReply::CiProfileInquiryReply (const Sysex7Message& msg)
: CiMessage { type.toString () }
{
    parseHeader (msg);
    const auto buf = msg.data.get ();
    if (!buf || buf->size () < kCepIdx + kCountLen)
        return;

    const size_t cep = static_cast<size_t> ((*buf)[kCepIdx]) |
                       (static_cast<size_t> ((*buf)[kCepIdx + 1]) << 7);
    const size_t enabledStart = kCepIdx + kCountLen;
    enabledProfiles = parseProfileList (*buf, enabledStart, cep);

    const size_t cdpIdx = enabledStart + cep * kProfileSize;
    if (cdpIdx + kCountLen > buf->size ())
        return;
    const size_t cdp = static_cast<size_t> ((*buf)[cdpIdx]) |
                       (static_cast<size_t> ((*buf)[cdpIdx + 1]) << 7);
    disabledProfiles = parseProfileList (*buf, cdpIdx + kCountLen, cdp);
}

CiProfileInquiryReply::CiProfileInquiryReply (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiProfileInquiryReply::CiProfileInquiryReply (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiProfileInquiryReply::CiProfileInquiryReply (MidiGroup theGroup, int sourceMuidValue, int destMuidValue)
: CiMessage { type.toString () }
{
    group         = theGroup.get () - 1;
    deviceId      = CiDeviceId::functionBlock;
    messageType   = CiType::profileInquiryReply;
    messageFormat = messageFormatMin;
    sourceMuid    = sourceMuidValue;
    destMuid      = destMuidValue;
}

Sysex7Message CiProfileInquiryReply::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, deviceId.get (), CiType::profileInquiryReply, targetFormat,
                        sourceMuid.get (), destMuid.get ());

    auto enabled  = enabledProfiles.get ();
    auto disabled = disabledProfiles.get ();

    const size_t cep = enabled  ? enabled->size ()  / kProfileSize : 0;
    const size_t cdp = disabled ? disabled->size () / kProfileSize : 0;

    buf->append (static_cast<uint8_t> (cep & 0x7F));
    buf->append (static_cast<uint8_t> ((cep >> 7) & 0x7F));
    if (enabled)
        for (size_t i = 0; i < enabled->size (); ++i)
            buf->append ((*enabled)[i]);

    buf->append (static_cast<uint8_t> (cdp & 0x7F));
    buf->append (static_cast<uint8_t> ((cdp >> 7) & 0x7F));
    if (disabled)
        for (size_t i = 0; i < disabled->size (); ++i)
            buf->append ((*disabled)[i]);

    return Sysex7Message { theGroup, buf };
}

void CiProfileInquiryReply::addEnabledProfile (ProfileId p)
{
    auto buf = enabledProfiles.get ();
    ensureBuffer (buf);
    enabledProfiles = buf;
    appendProfileId (*buf, p);
}

void CiProfileInquiryReply::addDisabledProfile (ProfileId p)
{
    auto buf = disabledProfiles.get ();
    ensureBuffer (buf);
    disabledProfiles = buf;
    appendProfileId (*buf, p);
}

int CiProfileInquiryReply::enabledProfileCount () const
{
    auto buf = enabledProfiles.get ();
    return buf ? static_cast<int> (buf->size () / kProfileSize) : 0;
}

int CiProfileInquiryReply::disabledProfileCount () const
{
    auto buf = disabledProfiles.get ();
    return buf ? static_cast<int> (buf->size () / kProfileSize) : 0;
}

ProfileId CiProfileInquiryReply::enabledProfileAt (int index) const
{
    auto buf = enabledProfiles.get ();
    if (!buf || index < 0 || index >= enabledProfileCount ())
        return {};
    return profileIdAt (*buf, static_cast<size_t> (index) * kProfileSize);
}

ProfileId CiProfileInquiryReply::disabledProfileAt (int index) const
{
    auto buf = disabledProfiles.get ();
    if (!buf || index < 0 || index >= disabledProfileCount ())
        return {};
    return profileIdAt (*buf, static_cast<size_t> (index) * kProfileSize);
}
