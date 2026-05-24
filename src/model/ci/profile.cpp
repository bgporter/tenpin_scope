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
constexpr size_t kCepIdx     = 13; // offset of enabled-profile count (2 bytes LSB first)
constexpr size_t kCountLen   = 2;  // byte width of cep / cdp fields
constexpr size_t kMinSize    = 13; // header only — profile lists may be empty
constexpr size_t kProfileIdx = 13; // offset of single ProfileId in Added/Removed messages
constexpr size_t kProfileSize = 5; // bytes per ProfileId on the wire

void appendCommonHeader (Buffer& buf, int devId, int msgType, int msgFmt, int src, int dst)
{
    CiSerial::appendCommonHeader (buf, msgType, msgFmt, src, dst, devId);
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

ProfileIdList::Ptr parseProfileList (const Buffer& buf, size_t offset, int count)
{
    if (count == 0)
        return {};
    ProfileIdList::Ptr list = new ProfileIdList ();
    for (int i = 0; i < count; ++i)
    {
        const size_t base = offset + static_cast<size_t> (i) * kProfileSize;
        if (base + 4 >= buf.size ())
            break;
        list->append (profileIdAt (buf, base));
    }
    return list;
}

void ensureList (ProfileIdList::Ptr& list)
{
    if (!list)
        list = new ProfileIdList ();
}
} // namespace

using CiSerial::appendMidiLong;
using CiSerial::parseMidiLong;

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

    const int cep = static_cast<int> ((*buf)[kCepIdx]) |
                    (static_cast<int> ((*buf)[kCepIdx + 1]) << 7);
    const size_t enabledStart = kCepIdx + kCountLen;
    enabledProfiles = parseProfileList (*buf, enabledStart, cep);

    const size_t cdpIdx = enabledStart + static_cast<size_t> (cep) * kProfileSize;
    if (cdpIdx + kCountLen > buf->size ())
        return;
    const int cdp = static_cast<int> ((*buf)[cdpIdx]) |
                    (static_cast<int> ((*buf)[cdpIdx + 1]) << 7);
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

    const int cep = enabled  ? enabled->size ()  : 0;
    const int cdp = disabled ? disabled->size () : 0;

    buf->append (static_cast<uint8_t> (cep & 0x7F));
    buf->append (static_cast<uint8_t> ((cep >> 7) & 0x7F));
    for (int i = 0; i < cep; ++i)
        appendProfileId (*buf, enabled->at (i));

    buf->append (static_cast<uint8_t> (cdp & 0x7F));
    buf->append (static_cast<uint8_t> ((cdp >> 7) & 0x7F));
    for (int i = 0; i < cdp; ++i)
        appendProfileId (*buf, disabled->at (i));

    return Sysex7Message { theGroup, buf };
}

void CiProfileInquiryReply::addEnabledProfile (ProfileId p)
{
    auto list = enabledProfiles.get ();
    ensureList (list);
    enabledProfiles = list;
    list->append (p);
}

void CiProfileInquiryReply::addDisabledProfile (ProfileId p)
{
    auto list = disabledProfiles.get ();
    ensureList (list);
    disabledProfiles = list;
    list->append (p);
}

int CiProfileInquiryReply::enabledProfileCount () const
{
    auto list = enabledProfiles.get ();
    return list ? list->size () : 0;
}

int CiProfileInquiryReply::disabledProfileCount () const
{
    auto list = disabledProfiles.get ();
    return list ? list->size () : 0;
}

ProfileId CiProfileInquiryReply::enabledProfileAt (int index) const
{
    auto list = enabledProfiles.get ();
    if (!list || index < 0 || index >= list->size ())
        return {};
    return list->at (index);
}

ProfileId CiProfileInquiryReply::disabledProfileAt (int index) const
{
    auto list = disabledProfiles.get ();
    if (!list || index < 0 || index >= list->size ())
        return {};
    return list->at (index);
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
    profile = profileIdAt (*buf, kSetProfileIdx);

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
    this->profile = profile;
    channelsRequested = channels;
}

Sysex7Message CiProfileSetOn::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, deviceId.get (), CiType::profileSetOn, targetFormat,
                        sourceMuid.get (), destMuid.get ());
    appendProfileId (*buf, profile.get ());
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
    profile = profileIdAt (*buf, kSetProfileIdx);
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
    this->profile = profile;
}

Sysex7Message CiProfileSetOff::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, deviceId.get (), CiType::profileSetOff, targetFormat,
                        sourceMuid.get (), destMuid.get ());
    appendProfileId (*buf, profile.get ());
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
    profile = profileIdAt (*buf, kReportProfileIdx);
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
    this->profile = profile;
    channelsEnabled = channels;
}

Sysex7Message CiProfileEnabled::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, deviceId.get (), CiType::profileEnabled, targetFormat,
                        sourceMuid.get (), destMuid.get ());
    appendProfileId (*buf, profile.get ());
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
    profile = profileIdAt (*buf, kReportProfileIdx);
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
    this->profile = profile;
    channelsDisabled = channels;
}

Sysex7Message CiProfileDisabled::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, deviceId.get (), CiType::profileDisabled, targetFormat,
                        sourceMuid.get (), destMuid.get ());
    appendProfileId (*buf, profile.get ());
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
    profile = profileIdAt (*buf, kProfileIdx);
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
    this->profile = profile;
}

Sysex7Message CiProfileAdded::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, deviceId.get (), CiType::profileAdded, targetFormat,
                        sourceMuid.get (), destMuid.get ());
    appendProfileId (*buf, profile.get ());
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
    profile = profileIdAt (*buf, kProfileIdx);
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
    this->profile = profile;
}

Sysex7Message CiProfileRemoved::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, deviceId.get (), CiType::profileRemoved, targetFormat,
                        sourceMuid.get (), destMuid.get ());
    appendProfileId (*buf, profile.get ());
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
    profile = profileIdAt (*buf, kDetailsProfileIdx);
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
    this->profile = profile;
    inquiryTarget = target.get ();
}

Sysex7Message CiProfileDetailsInquiry::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, deviceId.get (), CiType::profileDetailsInquiry, targetFormat,
                        sourceMuid.get (), destMuid.get ());
    appendProfileId (*buf, profile.get ());
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
    profile = profileIdAt (*buf, kDetailsProfileIdx);
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
    this->profile = profile;
    inquiryTarget = target.get ();
    targetData    = data;
}

Sysex7Message CiProfileDetailsInquiryReply::toSysex7Message (MidiNibble theGroup, int /*targetFormat*/) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, deviceId.get (), CiType::profileDetailsInquiryReply,
                        0x02, sourceMuid.get (), destMuid.get ());
    appendProfileId (*buf, profile.get ());
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
    profile = profileIdAt (*buf, kSpecificProfileIdx);

    const size_t dataLen = static_cast<size_t> (parseMidiLong (*buf, kSpecificDataLenIdx));

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
    this->profile = profile;
    profileData   = data;
}

Sysex7Message CiProfileSpecificData::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, deviceId.get (), CiType::profileSpecificData, targetFormat,
                        sourceMuid.get (), destMuid.get ());
    appendProfileId (*buf, profile.get ());

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
