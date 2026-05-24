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

#include <vector>

#include "model/midiTypes.h"

namespace CiDeviceId
{
constexpr int group         = 0x7E;
constexpr int functionBlock = 0x7F;
} // namespace CiDeviceId

namespace CiSubId
{
constexpr int midiCi = 0x0D;
} // namespace CiSubId

namespace CiType
{
constexpr int peCapabilitiesInquiry      = 0x30;
constexpr int peCapabilitiesReply        = 0x31;
constexpr int peGetPropertyDataInquiry   = 0x34;
constexpr int peGetPropertyDataReply     = 0x35;
constexpr int peSetPropertyDataInquiry   = 0x36;
constexpr int peSetPropertyDataReply     = 0x37;
constexpr int peSubscriptionInquiry      = 0x38;
constexpr int peSubscriptionReply        = 0x39;
constexpr int peNotify                   = 0x3F; // deprecated in MIDI-CI v1.1; use ACK/NAK instead
constexpr int profileInquiry             = 0x20;
constexpr int profileInquiryReply        = 0x21;
constexpr int profileSetOn               = 0x22;
constexpr int profileSetOff              = 0x23;
constexpr int profileEnabled             = 0x24;
constexpr int profileDisabled            = 0x25;
constexpr int profileAdded               = 0x26;
constexpr int profileRemoved             = 0x27;
constexpr int profileDetailsInquiry      = 0x28;
constexpr int profileDetailsInquiryReply = 0x29;
constexpr int profileSpecificData        = 0x2F;
constexpr int discoveryInquiry    = 0x70;
constexpr int discoveryReply    = 0x71;
constexpr int endpointInquiry   = 0x72;
constexpr int endpointReply     = 0x73;
constexpr int invalidateMuid    = 0x7E;
constexpr int nak               = 0x7F;
constexpr int ack               = 0x7D;
} // namespace CiType

namespace CiAckStatus
{
constexpr int ack               = 0x00; // success
constexpr int timeout           = 0x10; // status data = wait time in 100ms units
constexpr int flowControl       = 0x11; // send next chunks
} // namespace CiAckStatus

namespace CiNakStatus
{
constexpr int nak                       = 0x00; // generic NAK
constexpr int messageNotSupported       = 0x01;
constexpr int versionNotSupported       = 0x02;
constexpr int channelNotInUse          = 0x03;
constexpr int profileNotSupported       = 0x04;
constexpr int flowControlResend         = 0x12; // resend most recent chunk
constexpr int responderTerminates       = 0x20;
constexpr int retryError               = 0x40;
constexpr int malformed                = 0x41;
constexpr int timeout                  = 0x42;
constexpr int busy                     = 0x43; // status data = wait time in 100ms units
} // namespace CiNakStatus

namespace CiEndpointStatus
{
constexpr int productInstanceId = 0x00;
} // namespace CiEndpointStatus

namespace CiCategory
{
constexpr int protocolNegotiation  = 0x02;
constexpr int profileConfiguration = 0x04;
constexpr int propertyExchange     = 0x08;
constexpr int processInquiry       = 0x10;
} // namespace CiCategory

constexpr int broadcastMuid       = MidiLong::maxValue; // 0x0FFFFFFF
constexpr int messageFormatMin    = 0x01;               // format 0x00 is disallowed
constexpr int messageFormatLatest = 0x02;               // highest format version implemented

// ---------------------------------------------------------------------------

/** A 5-byte MIDI-CI Profile ID. */
struct ProfileId
{
    uint8_t byte1 { 0 };
    uint8_t byte2 { 0 };
    uint8_t byte3 { 0 };
    uint8_t byte4 { 0 };
    uint8_t byte5 { 0 };

    bool isStandardDefined () const { return byte1 == 0x7E; }

    bool operator== (const ProfileId&) const = default;
    bool operator!= (const ProfileId&) const = default;
};

namespace juce
{
template <> struct VariantConverter<ProfileId>
{
    static ProfileId fromVar (const juce::var& v)
    {
        auto t = juce::StringArray::fromTokens (v.toString (), ",", "");
        if (t.size () < 5)
            return {};
        return { static_cast<uint8_t> (t[0].getHexValue32 ()),
                 static_cast<uint8_t> (t[1].getHexValue32 ()),
                 static_cast<uint8_t> (t[2].getHexValue32 ()),
                 static_cast<uint8_t> (t[3].getHexValue32 ()),
                 static_cast<uint8_t> (t[4].getHexValue32 ()) };
    }

    static juce::var toVar (ProfileId p)
    {
        auto hex = [] (uint8_t b)
        { return juce::String::toHexString (static_cast<int> (b)).paddedLeft ('0', 2).toUpperCase (); };
        return hex (p.byte1) + "," + hex (p.byte2) + "," + hex (p.byte3) + "," +
               hex (p.byte4) + "," + hex (p.byte5);
    }
};
} // namespace juce

// ---------------------------------------------------------------------------

/**
 * An ordered list of ProfileId values, storable in a juce::ValueTree via
 * MAKE_VALUE_MEMBER (ProfileIdList::Ptr, ..., {}).
 */
class ProfileIdList : public juce::ReferenceCountedObject
{
public:
    using Ptr = juce::ReferenceCountedObjectPtr<ProfileIdList>;

    ProfileIdList () = default;

    void      append (ProfileId p)       { ids_.push_back (p); }
    int       size ()            const   { return static_cast<int> (ids_.size ()); }
    bool      empty ()           const   { return ids_.empty (); }
    ProfileId at (int index)     const   { return ids_[static_cast<size_t> (index)]; }

    auto begin () const { return ids_.begin (); }
    auto end ()   const { return ids_.end (); }

private:
    std::vector<ProfileId> ids_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProfileIdList)
};

namespace juce
{
template <> struct VariantConverter<ProfileIdList::Ptr>
{
    static ProfileIdList::Ptr fromVar (const juce::var& v)
    {
        return ProfileIdList::Ptr (static_cast<ProfileIdList*> (v.getObject ()));
    }
    static juce::var toVar (ProfileIdList::Ptr ptr) { return juce::var (ptr.get ()); }
};
} // namespace juce
