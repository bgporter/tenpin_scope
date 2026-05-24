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
constexpr size_t kCepIdx      = 13; // offset of enabled-profile count (2 bytes LSB first)
constexpr size_t kCountLen    = 2;  // byte width of cep / cdp fields
constexpr size_t kMinSize     = 13; // header only — profile lists may be empty
constexpr size_t kProfileIdx  = 13; // offset of single ProfileId in Added/Removed messages
constexpr size_t kProfileSize = CiProfileInquiryReply::kProfileSize;

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

// ============================================================================
// CiProfileSetOn
// ============================================================================

namespace
{
constexpr size_t kSetProfileIdx         = 13;
constexpr size_t kChannelsRequestedIdx  = 18;
constexpr size_t kSetMinSize            = 18; // header(13) + profileId(5)
} // namespace

CiProfileSetOn::CiProfileSetOn (const Sysex7Message& msg)
: CiMessage { type.toString () }
{
    parseHeader (msg);
    const auto buf = msg.data.get ();
    if (!buf || buf->size () < kSetMinSize)
        return;
    profileByte1 = static_cast<int> ((*buf)[kSetProfileIdx]);
    profileByte2 = static_cast<int> ((*buf)[kSetProfileIdx + 1]);
    profileByte3 = static_cast<int> ((*buf)[kSetProfileIdx + 2]);
    profileByte4 = static_cast<int> ((*buf)[kSetProfileIdx + 3]);
    profileByte5 = static_cast<int> ((*buf)[kSetProfileIdx + 4]);

    if (buf->size () >= kChannelsRequestedIdx + 2)
        channelsRequested = static_cast<int> ((*buf)[kChannelsRequestedIdx]) |
                            (static_cast<int> ((*buf)[kChannelsRequestedIdx + 1]) << 7);
}

CiProfileSetOn::CiProfileSetOn (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiProfileSetOn::CiProfileSetOn (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiProfileSetOn::CiProfileSetOn (MidiGroup theGroup, int sourceMuidValue, int destMuidValue,
                                 ProfileId profile, int channels)
: CiMessage { type.toString () }
{
    group             = theGroup.get () - 1;
    deviceId          = CiDeviceId::functionBlock;
    messageType       = CiType::profileSetOn;
    messageFormat     = messageFormatMin;
    sourceMuid        = sourceMuidValue;
    destMuid          = destMuidValue;
    profileByte1      = profile.byte1;
    profileByte2      = profile.byte2;
    profileByte3      = profile.byte3;
    profileByte4      = profile.byte4;
    profileByte5      = profile.byte5;
    channelsRequested = channels;
}

Sysex7Message CiProfileSetOn::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, deviceId.get (), CiType::profileSetOn, targetFormat,
                        sourceMuid.get (), destMuid.get ());
    buf->append (static_cast<uint8_t> (profileByte1.get ()));
    buf->append (static_cast<uint8_t> (profileByte2.get ()));
    buf->append (static_cast<uint8_t> (profileByte3.get ()));
    buf->append (static_cast<uint8_t> (profileByte4.get ()));
    buf->append (static_cast<uint8_t> (profileByte5.get ()));
    if (targetFormat >= 2)
    {
        const int ch = channelsRequested.get ();
        buf->append (static_cast<uint8_t> (ch & 0x7F));
        buf->append (static_cast<uint8_t> ((ch >> 7) & 0x7F));
    }
    return Sysex7Message { theGroup, buf };
}

// ============================================================================
// CiProfileSetOff
// ============================================================================

CiProfileSetOff::CiProfileSetOff (const Sysex7Message& msg)
: CiMessage { type.toString () }
{
    parseHeader (msg);
    const auto buf = msg.data.get ();
    if (!buf || buf->size () < kSetMinSize)
        return;
    profileByte1 = static_cast<int> ((*buf)[kSetProfileIdx]);
    profileByte2 = static_cast<int> ((*buf)[kSetProfileIdx + 1]);
    profileByte3 = static_cast<int> ((*buf)[kSetProfileIdx + 2]);
    profileByte4 = static_cast<int> ((*buf)[kSetProfileIdx + 3]);
    profileByte5 = static_cast<int> ((*buf)[kSetProfileIdx + 4]);
}

CiProfileSetOff::CiProfileSetOff (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiProfileSetOff::CiProfileSetOff (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiProfileSetOff::CiProfileSetOff (MidiGroup theGroup, int sourceMuidValue, int destMuidValue,
                                   ProfileId profile)
: CiMessage { type.toString () }
{
    group         = theGroup.get () - 1;
    deviceId      = CiDeviceId::functionBlock;
    messageType   = CiType::profileSetOff;
    messageFormat = messageFormatMin;
    sourceMuid    = sourceMuidValue;
    destMuid      = destMuidValue;
    profileByte1  = profile.byte1;
    profileByte2  = profile.byte2;
    profileByte3  = profile.byte3;
    profileByte4  = profile.byte4;
    profileByte5  = profile.byte5;
}

Sysex7Message CiProfileSetOff::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, deviceId.get (), CiType::profileSetOff, targetFormat,
                        sourceMuid.get (), destMuid.get ());
    buf->append (static_cast<uint8_t> (profileByte1.get ()));
    buf->append (static_cast<uint8_t> (profileByte2.get ()));
    buf->append (static_cast<uint8_t> (profileByte3.get ()));
    buf->append (static_cast<uint8_t> (profileByte4.get ()));
    buf->append (static_cast<uint8_t> (profileByte5.get ()));
    if (targetFormat >= 2)
    {
        buf->append (0x00); // reserved
        buf->append (0x00);
    }
    return Sysex7Message { theGroup, buf };
}

// ============================================================================
// CiProfileEnabled / CiProfileDisabled  (shared layout helpers)
// ============================================================================

namespace
{
constexpr size_t kReportProfileIdx    = 13;
constexpr size_t kReportChannelsIdx   = 18;
constexpr size_t kReportMinSize       = 18; // header(13) + profileId(5)

void parseReportProfile (CiMessage& msg, const Buffer* buf,
                         int& b1, int& b2, int& b3, int& b4, int& b5)
{
    b1 = static_cast<int> ((*buf)[kReportProfileIdx]);
    b2 = static_cast<int> ((*buf)[kReportProfileIdx + 1]);
    b3 = static_cast<int> ((*buf)[kReportProfileIdx + 2]);
    b4 = static_cast<int> ((*buf)[kReportProfileIdx + 3]);
    b5 = static_cast<int> ((*buf)[kReportProfileIdx + 4]);
}
} // namespace

// ============================================================================
// CiProfileEnabled
// ============================================================================

CiProfileEnabled::CiProfileEnabled (const Sysex7Message& msg)
: CiMessage { type.toString () }
{
    parseHeader (msg);
    const auto buf = msg.data.get ();
    if (!buf || buf->size () < kReportMinSize)
        return;
    int b1, b2, b3, b4, b5;
    parseReportProfile (*this, buf, b1, b2, b3, b4, b5);
    profileByte1 = b1; profileByte2 = b2; profileByte3 = b3;
    profileByte4 = b4; profileByte5 = b5;
    if (buf->size () >= kReportChannelsIdx + 2)
        channelsEnabled = static_cast<int> ((*buf)[kReportChannelsIdx]) |
                          (static_cast<int> ((*buf)[kReportChannelsIdx + 1]) << 7);
}

CiProfileEnabled::CiProfileEnabled (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiProfileEnabled::CiProfileEnabled (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiProfileEnabled::CiProfileEnabled (MidiGroup theGroup, int sourceMuidValue,
                                     ProfileId profile, int channels)
: CiMessage { type.toString () }
{
    group           = theGroup.get () - 1;
    deviceId        = CiDeviceId::functionBlock;
    messageType     = CiType::profileEnabled;
    messageFormat   = messageFormatMin;
    sourceMuid      = sourceMuidValue;
    destMuid        = broadcastMuid;
    profileByte1    = profile.byte1;
    profileByte2    = profile.byte2;
    profileByte3    = profile.byte3;
    profileByte4    = profile.byte4;
    profileByte5    = profile.byte5;
    channelsEnabled = channels;
}

Sysex7Message CiProfileEnabled::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, deviceId.get (), CiType::profileEnabled, targetFormat,
                        sourceMuid.get (), destMuid.get ());
    buf->append (static_cast<uint8_t> (profileByte1.get ()));
    buf->append (static_cast<uint8_t> (profileByte2.get ()));
    buf->append (static_cast<uint8_t> (profileByte3.get ()));
    buf->append (static_cast<uint8_t> (profileByte4.get ()));
    buf->append (static_cast<uint8_t> (profileByte5.get ()));
    if (targetFormat >= 2)
    {
        const int ch = channelsEnabled.get ();
        buf->append (static_cast<uint8_t> (ch & 0x7F));
        buf->append (static_cast<uint8_t> ((ch >> 7) & 0x7F));
    }
    return Sysex7Message { theGroup, buf };
}

// ============================================================================
// CiProfileDisabled
// ============================================================================

CiProfileDisabled::CiProfileDisabled (const Sysex7Message& msg)
: CiMessage { type.toString () }
{
    parseHeader (msg);
    const auto buf = msg.data.get ();
    if (!buf || buf->size () < kReportMinSize)
        return;
    int b1, b2, b3, b4, b5;
    parseReportProfile (*this, buf, b1, b2, b3, b4, b5);
    profileByte1 = b1; profileByte2 = b2; profileByte3 = b3;
    profileByte4 = b4; profileByte5 = b5;
    if (buf->size () >= kReportChannelsIdx + 2)
        channelsDisabled = static_cast<int> ((*buf)[kReportChannelsIdx]) |
                           (static_cast<int> ((*buf)[kReportChannelsIdx + 1]) << 7);
}

CiProfileDisabled::CiProfileDisabled (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiProfileDisabled::CiProfileDisabled (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiProfileDisabled::CiProfileDisabled (MidiGroup theGroup, int sourceMuidValue,
                                       ProfileId profile, int channels)
: CiMessage { type.toString () }
{
    group            = theGroup.get () - 1;
    deviceId         = CiDeviceId::functionBlock;
    messageType      = CiType::profileDisabled;
    messageFormat    = messageFormatMin;
    sourceMuid       = sourceMuidValue;
    destMuid         = broadcastMuid;
    profileByte1     = profile.byte1;
    profileByte2     = profile.byte2;
    profileByte3     = profile.byte3;
    profileByte4     = profile.byte4;
    profileByte5     = profile.byte5;
    channelsDisabled = channels;
}

Sysex7Message CiProfileDisabled::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, deviceId.get (), CiType::profileDisabled, targetFormat,
                        sourceMuid.get (), destMuid.get ());
    buf->append (static_cast<uint8_t> (profileByte1.get ()));
    buf->append (static_cast<uint8_t> (profileByte2.get ()));
    buf->append (static_cast<uint8_t> (profileByte3.get ()));
    buf->append (static_cast<uint8_t> (profileByte4.get ()));
    buf->append (static_cast<uint8_t> (profileByte5.get ()));
    if (targetFormat >= 2)
    {
        const int ch = channelsDisabled.get ();
        buf->append (static_cast<uint8_t> (ch & 0x7F));
        buf->append (static_cast<uint8_t> ((ch >> 7) & 0x7F));
    }
    return Sysex7Message { theGroup, buf };
}

// ============================================================================
// CiProfileAdded
// ============================================================================

CiProfileAdded::CiProfileAdded (const Sysex7Message& msg)
: CiMessage { type.toString () }
{
    parseHeader (msg);
    const auto buf = msg.data.get ();
    if (!buf || buf->size () < kProfileIdx + kProfileSize)
        return;
    profileByte1 = static_cast<int> ((*buf)[kProfileIdx]);
    profileByte2 = static_cast<int> ((*buf)[kProfileIdx + 1]);
    profileByte3 = static_cast<int> ((*buf)[kProfileIdx + 2]);
    profileByte4 = static_cast<int> ((*buf)[kProfileIdx + 3]);
    profileByte5 = static_cast<int> ((*buf)[kProfileIdx + 4]);
}

CiProfileAdded::CiProfileAdded (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiProfileAdded::CiProfileAdded (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiProfileAdded::CiProfileAdded (MidiGroup theGroup, int sourceMuidValue, ProfileId profile)
: CiMessage { type.toString () }
{
    group         = theGroup.get () - 1;
    deviceId      = CiDeviceId::functionBlock;
    messageType   = CiType::profileAdded;
    messageFormat = messageFormatMin;
    sourceMuid    = sourceMuidValue;
    destMuid      = broadcastMuid;
    profileByte1  = profile.byte1;
    profileByte2  = profile.byte2;
    profileByte3  = profile.byte3;
    profileByte4  = profile.byte4;
    profileByte5  = profile.byte5;
}

Sysex7Message CiProfileAdded::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, deviceId.get (), CiType::profileAdded, targetFormat,
                        sourceMuid.get (), destMuid.get ());
    buf->append (static_cast<uint8_t> (profileByte1.get ()));
    buf->append (static_cast<uint8_t> (profileByte2.get ()));
    buf->append (static_cast<uint8_t> (profileByte3.get ()));
    buf->append (static_cast<uint8_t> (profileByte4.get ()));
    buf->append (static_cast<uint8_t> (profileByte5.get ()));
    return Sysex7Message { theGroup, buf };
}

// ============================================================================
// CiProfileRemoved
// ============================================================================

CiProfileRemoved::CiProfileRemoved (const Sysex7Message& msg)
: CiMessage { type.toString () }
{
    parseHeader (msg);
    const auto buf = msg.data.get ();
    if (!buf || buf->size () < kProfileIdx + kProfileSize)
        return;
    profileByte1 = static_cast<int> ((*buf)[kProfileIdx]);
    profileByte2 = static_cast<int> ((*buf)[kProfileIdx + 1]);
    profileByte3 = static_cast<int> ((*buf)[kProfileIdx + 2]);
    profileByte4 = static_cast<int> ((*buf)[kProfileIdx + 3]);
    profileByte5 = static_cast<int> ((*buf)[kProfileIdx + 4]);
}

CiProfileRemoved::CiProfileRemoved (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiProfileRemoved::CiProfileRemoved (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiProfileRemoved::CiProfileRemoved (MidiGroup theGroup, int sourceMuidValue, ProfileId profile)
: CiMessage { type.toString () }
{
    group         = theGroup.get () - 1;
    deviceId      = CiDeviceId::functionBlock;
    messageType   = CiType::profileRemoved;
    messageFormat = messageFormatMin;
    sourceMuid    = sourceMuidValue;
    destMuid      = broadcastMuid;
    profileByte1  = profile.byte1;
    profileByte2  = profile.byte2;
    profileByte3  = profile.byte3;
    profileByte4  = profile.byte4;
    profileByte5  = profile.byte5;
}

Sysex7Message CiProfileRemoved::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, deviceId.get (), CiType::profileRemoved, targetFormat,
                        sourceMuid.get (), destMuid.get ());
    buf->append (static_cast<uint8_t> (profileByte1.get ()));
    buf->append (static_cast<uint8_t> (profileByte2.get ()));
    buf->append (static_cast<uint8_t> (profileByte3.get ()));
    buf->append (static_cast<uint8_t> (profileByte4.get ()));
    buf->append (static_cast<uint8_t> (profileByte5.get ()));
    return Sysex7Message { theGroup, buf };
}

// ============================================================================
// CiProfileDetailsInquiry
// ============================================================================

namespace
{
constexpr size_t kDetailsProfileIdx   = 13;
constexpr size_t kInquiryTargetIdx    = 18;
constexpr size_t kDetailsMinSizeInq   = 19; // header(13) + profileId(5) + target(1)
constexpr size_t kTargetDataLenIdx    = 19; // reply: dl at [19..20]
constexpr size_t kTargetDataStart     = 21;
constexpr size_t kDetailsMinSizeReply = 21; // header(13) + profileId(5) + target(1) + dl(2)
} // namespace

CiProfileDetailsInquiry::CiProfileDetailsInquiry (const Sysex7Message& msg)
: CiMessage { type.toString () }
{
    parseHeader (msg);
    const auto buf = msg.data.get ();
    if (!buf || buf->size () < kDetailsMinSizeInq)
        return;
    profileByte1  = static_cast<int> ((*buf)[kDetailsProfileIdx]);
    profileByte2  = static_cast<int> ((*buf)[kDetailsProfileIdx + 1]);
    profileByte3  = static_cast<int> ((*buf)[kDetailsProfileIdx + 2]);
    profileByte4  = static_cast<int> ((*buf)[kDetailsProfileIdx + 3]);
    profileByte5  = static_cast<int> ((*buf)[kDetailsProfileIdx + 4]);
    inquiryTarget = static_cast<int> ((*buf)[kInquiryTargetIdx]);
}

CiProfileDetailsInquiry::CiProfileDetailsInquiry (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiProfileDetailsInquiry::CiProfileDetailsInquiry (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiProfileDetailsInquiry::CiProfileDetailsInquiry (MidiGroup theGroup, int sourceMuidValue,
                                                   int destMuidValue, ProfileId profile,
                                                   MidiByte target)
: CiMessage { type.toString () }
{
    group         = theGroup.get () - 1;
    deviceId      = CiDeviceId::functionBlock;
    messageType   = CiType::profileDetailsInquiry;
    messageFormat = messageFormatMin;
    sourceMuid    = sourceMuidValue;
    destMuid      = destMuidValue;
    profileByte1  = profile.byte1;
    profileByte2  = profile.byte2;
    profileByte3  = profile.byte3;
    profileByte4  = profile.byte4;
    profileByte5  = profile.byte5;
    inquiryTarget = target.get ();
}

Sysex7Message CiProfileDetailsInquiry::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, deviceId.get (), CiType::profileDetailsInquiry, targetFormat,
                        sourceMuid.get (), destMuid.get ());
    buf->append (static_cast<uint8_t> (profileByte1.get ()));
    buf->append (static_cast<uint8_t> (profileByte2.get ()));
    buf->append (static_cast<uint8_t> (profileByte3.get ()));
    buf->append (static_cast<uint8_t> (profileByte4.get ()));
    buf->append (static_cast<uint8_t> (profileByte5.get ()));
    buf->append (static_cast<uint8_t> (inquiryTarget.get ()));
    return Sysex7Message { theGroup, buf };
}

// ============================================================================
// CiProfileDetailsInquiryReply
// ============================================================================

CiProfileDetailsInquiryReply::CiProfileDetailsInquiryReply (const Sysex7Message& msg)
: CiMessage { type.toString () }
{
    parseHeader (msg);
    const auto buf = msg.data.get ();
    if (!buf || buf->size () < kDetailsMinSizeReply)
        return;
    profileByte1  = static_cast<int> ((*buf)[kDetailsProfileIdx]);
    profileByte2  = static_cast<int> ((*buf)[kDetailsProfileIdx + 1]);
    profileByte3  = static_cast<int> ((*buf)[kDetailsProfileIdx + 2]);
    profileByte4  = static_cast<int> ((*buf)[kDetailsProfileIdx + 3]);
    profileByte5  = static_cast<int> ((*buf)[kDetailsProfileIdx + 4]);
    inquiryTarget = static_cast<int> ((*buf)[kInquiryTargetIdx]);

    const size_t dl = static_cast<size_t> ((*buf)[kTargetDataLenIdx]) |
                      (static_cast<size_t> ((*buf)[kTargetDataLenIdx + 1]) << 7);
    if (dl > 0 && kTargetDataStart + dl <= buf->size ())
    {
        Buffer::Ptr data = new Buffer ();
        for (size_t i = 0; i < dl; ++i)
            data->append ((*buf)[kTargetDataStart + i]);
        targetData = data;
    }
}

CiProfileDetailsInquiryReply::CiProfileDetailsInquiryReply (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiProfileDetailsInquiryReply::CiProfileDetailsInquiryReply (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiProfileDetailsInquiryReply::CiProfileDetailsInquiryReply (MidiGroup theGroup, int sourceMuidValue,
                                                             int destMuidValue, ProfileId profile,
                                                             MidiByte target, Buffer::Ptr data)
: CiMessage { type.toString () }
{
    group         = theGroup.get () - 1;
    deviceId      = CiDeviceId::functionBlock;
    messageType   = CiType::profileDetailsInquiryReply;
    messageFormat = 0x02; // spec mandates format 2
    sourceMuid    = sourceMuidValue;
    destMuid      = destMuidValue;
    profileByte1  = profile.byte1;
    profileByte2  = profile.byte2;
    profileByte3  = profile.byte3;
    profileByte4  = profile.byte4;
    profileByte5  = profile.byte5;
    inquiryTarget = target.get ();
    targetData    = data;
}

Sysex7Message CiProfileDetailsInquiryReply::toSysex7Message (MidiNibble theGroup, int /*targetFormat*/) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, deviceId.get (), CiType::profileDetailsInquiryReply,
                        0x02, sourceMuid.get (), destMuid.get ());
    buf->append (static_cast<uint8_t> (profileByte1.get ()));
    buf->append (static_cast<uint8_t> (profileByte2.get ()));
    buf->append (static_cast<uint8_t> (profileByte3.get ()));
    buf->append (static_cast<uint8_t> (profileByte4.get ()));
    buf->append (static_cast<uint8_t> (profileByte5.get ()));
    buf->append (static_cast<uint8_t> (inquiryTarget.get ()));

    auto data       = targetData.get ();
    const size_t dl = data ? data->size () : 0;
    buf->append (static_cast<uint8_t> (dl & 0x7F));
    buf->append (static_cast<uint8_t> ((dl >> 7) & 0x7F));
    for (size_t i = 0; i < dl; ++i)
        buf->append ((*data)[i]);

    return Sysex7Message { theGroup, buf };
}

// ============================================================================
// CiProfileSpecificData
// ============================================================================

namespace
{
constexpr size_t kSpecificProfileIdx  = 13;
constexpr size_t kSpecificDataLenIdx  = 18; // 4-byte MidiLong length field
constexpr size_t kSpecificDataStart   = 22;
constexpr size_t kSpecificMinSize     = 22; // header(13) + profileId(5) + length(4)
} // namespace

CiProfileSpecificData::CiProfileSpecificData (const Sysex7Message& msg)
: CiMessage { type.toString () }
{
    parseHeader (msg);
    const auto buf = msg.data.get ();
    if (!buf || buf->size () < kSpecificMinSize)
        return;
    profileByte1 = static_cast<int> ((*buf)[kSpecificProfileIdx]);
    profileByte2 = static_cast<int> ((*buf)[kSpecificProfileIdx + 1]);
    profileByte3 = static_cast<int> ((*buf)[kSpecificProfileIdx + 2]);
    profileByte4 = static_cast<int> ((*buf)[kSpecificProfileIdx + 3]);
    profileByte5 = static_cast<int> ((*buf)[kSpecificProfileIdx + 4]);

    const size_t dataLen = static_cast<size_t> (
        MidiLong { static_cast<int> ((*buf)[kSpecificDataLenIdx]),
                   static_cast<int> ((*buf)[kSpecificDataLenIdx + 1]),
                   static_cast<int> ((*buf)[kSpecificDataLenIdx + 2]),
                   static_cast<int> ((*buf)[kSpecificDataLenIdx + 3]) }.get ());

    if (dataLen > 0 && kSpecificDataStart + dataLen <= buf->size ())
    {
        Buffer::Ptr data = new Buffer ();
        for (size_t i = 0; i < dataLen; ++i)
            data->append ((*buf)[kSpecificDataStart + i]);
        profileData = data;
    }
}

CiProfileSpecificData::CiProfileSpecificData (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiProfileSpecificData::CiProfileSpecificData (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiProfileSpecificData::CiProfileSpecificData (MidiGroup theGroup, int sourceMuidValue,
                                               int destMuidValue, ProfileId profile,
                                               Buffer::Ptr data)
: CiMessage { type.toString () }
{
    group         = theGroup.get () - 1;
    deviceId      = CiDeviceId::functionBlock;
    messageType   = CiType::profileSpecificData;
    messageFormat = messageFormatMin;
    sourceMuid    = sourceMuidValue;
    destMuid      = destMuidValue;
    profileByte1  = profile.byte1;
    profileByte2  = profile.byte2;
    profileByte3  = profile.byte3;
    profileByte4  = profile.byte4;
    profileByte5  = profile.byte5;
    profileData   = data;
}

Sysex7Message CiProfileSpecificData::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, deviceId.get (), CiType::profileSpecificData, targetFormat,
                        sourceMuid.get (), destMuid.get ());
    buf->append (static_cast<uint8_t> (profileByte1.get ()));
    buf->append (static_cast<uint8_t> (profileByte2.get ()));
    buf->append (static_cast<uint8_t> (profileByte3.get ()));
    buf->append (static_cast<uint8_t> (profileByte4.get ()));
    buf->append (static_cast<uint8_t> (profileByte5.get ()));

    auto data           = profileData.get ();
    const int dataLen   = data ? static_cast<int> (data->size ()) : 0;
    MidiLong lenField { dataLen };
    buf->append (static_cast<uint8_t> (lenField.getLsb ()));
    buf->append (static_cast<uint8_t> (lenField.getByte2 ()));
    buf->append (static_cast<uint8_t> (lenField.getByte3 ()));
    buf->append (static_cast<uint8_t> (lenField.getMsb ()));
    for (int i = 0; i < dataLen; ++i)
        buf->append ((*data)[static_cast<size_t> (i)]);

    return Sysex7Message { theGroup, buf };
}

#if RUN_UNIT_TESTS
#include "test/test_CiProfile.inl"
#endif
