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

#include "model/ci/ciMessage.h"
#include "model/midiTypes.h"
#include "model/sysex/sysex7Message.h"

// ---------------------------------------------------------------------------

/**
 * @brief CI Profile Inquiry (0x20).
 *
 * Asks a responder for the list of profiles it supports.
 * No payload beyond the common CI header.
 */
struct CiProfileInquiry : public CiMessage
{
    static const inline juce::Identifier type { "CiProfileInquiry" };

    explicit CiProfileInquiry (const Sysex7Message& msg);
    explicit CiProfileInquiry (const Event& e);
    explicit CiProfileInquiry (juce::ValueTree vt);

    // Programmatic construction.
    CiProfileInquiry (MidiGroup group, int sourceMuid, int destMuid);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;
};

// ---------------------------------------------------------------------------

/**
 * @brief CI Reply to Profile Inquiry (0x21).
 *
 * Wire format payload (after common CI header at buf[13]):
 *   [13..14]               cep: number of currently enabled profiles (LSB first)
 *   [15..15+cep*5-1]       enabled profile IDs (cep × 5 bytes each)
 *   [15+cep*5..+1]         cdp: number of currently disabled profiles (LSB first)
 *   [15+cep*5+2..]         disabled profile IDs (cdp × 5 bytes each)
 *
 * Profiles are stored as packed 5-byte sequences in enabledProfiles /
 * disabledProfiles buffers. Use addEnabledProfile / addDisabledProfile to
 * build the message programmatically; cep and cdp are derived at serialize time.
 */
struct CiProfileInquiryReply : public CiMessage
{
    static const inline juce::Identifier type { "CiProfileInquiryReply" };

    explicit CiProfileInquiryReply (const Sysex7Message& msg);
    explicit CiProfileInquiryReply (const Event& e);
    explicit CiProfileInquiryReply (juce::ValueTree vt);

    // Programmatic construction — add profiles after construction.
    CiProfileInquiryReply (MidiGroup group, int sourceMuid, int destMuid);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    void addEnabledProfile  (ProfileId p);
    void addDisabledProfile (ProfileId p);

    int       enabledProfileCount  () const;
    int       disabledProfileCount () const;
    ProfileId enabledProfileAt     (int index) const;
    ProfileId disabledProfileAt    (int index) const;

    MAKE_VALUE_MEMBER (ProfileIdList::Ptr, enabledProfiles,  {});
    MAKE_VALUE_MEMBER (ProfileIdList::Ptr, disabledProfiles, {});
};

// ---------------------------------------------------------------------------

/**
 * @brief CI Set Profile On (0x22). One-way command — no reply.
 *
 * Sent by an Initiator to enable a Profile on a Responder.
 * Payload (after common CI header at buf[13]):
 *   [13..17] Profile ID (5 bytes)
 *   [18..19] channelsRequested (v2+, LSB first); 0x0000 for group/function-block targets
 */
struct CiProfileSetOn : public CiMessage
{
    static const inline juce::Identifier type { "CiProfileSetOn" };

    explicit CiProfileSetOn (const Sysex7Message& msg);
    explicit CiProfileSetOn (const Event& e);
    explicit CiProfileSetOn (juce::ValueTree vt);

    CiProfileSetOn (MidiGroup group, int sourceMuid, int destMuid,
                    ProfileId profile, int channelsRequested = 0);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    MAKE_VALUE_MEMBER (ProfileId, profile,          {});
    MAKE_VALUE_MEMBER (int,       channelsRequested, 0);
};

// ---------------------------------------------------------------------------

/**
 * @brief CI Set Profile Off (0x23). One-way command — no reply.
 *
 * Sent by an Initiator to disable a Profile on a Responder.
 * Payload (after common CI header at buf[13]):
 *   [13..17] Profile ID (5 bytes)
 *   [18..19] Reserved (v2+, always 0x0000)
 */
struct CiProfileSetOff : public CiMessage
{
    static const inline juce::Identifier type { "CiProfileSetOff" };

    explicit CiProfileSetOff (const Sysex7Message& msg);
    explicit CiProfileSetOff (const Event& e);
    explicit CiProfileSetOff (juce::ValueTree vt);

    CiProfileSetOff (MidiGroup group, int sourceMuid, int destMuid, ProfileId profile);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    MAKE_VALUE_MEMBER (ProfileId, profile, {});
};

// ---------------------------------------------------------------------------

/**
 * @brief CI Profile Enabled Report (0x24). One-way broadcast.
 *
 * Sent by a device when a Profile becomes enabled. Destination MUID is
 * always broadcast. Payload (after common CI header at buf[13]):
 *   [13..17] Profile ID (5 bytes)
 *   [18..19] channelsEnabled (v2+, LSB first); 0x0000 for group/function-block targets
 */
struct CiProfileEnabled : public CiMessage
{
    static const inline juce::Identifier type { "CiProfileEnabled" };

    explicit CiProfileEnabled (const Sysex7Message& msg);
    explicit CiProfileEnabled (const Event& e);
    explicit CiProfileEnabled (juce::ValueTree vt);

    CiProfileEnabled (MidiGroup group, int sourceMuid, ProfileId profile,
                      int channelsEnabled = 0);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    MAKE_VALUE_MEMBER (ProfileId, profile,        {});
    MAKE_VALUE_MEMBER (int,       channelsEnabled, 0);
};

// ---------------------------------------------------------------------------

/**
 * @brief CI Profile Disabled Report (0x25). One-way broadcast.
 *
 * Sent by a device when a Profile becomes disabled. Destination MUID is
 * always broadcast. Payload (after common CI header at buf[13]):
 *   [13..17] Profile ID (5 bytes)
 *   [18..19] channelsDisabled (v2+, LSB first); 0x0000 for group/function-block targets
 */
struct CiProfileDisabled : public CiMessage
{
    static const inline juce::Identifier type { "CiProfileDisabled" };

    explicit CiProfileDisabled (const Sysex7Message& msg);
    explicit CiProfileDisabled (const Event& e);
    explicit CiProfileDisabled (juce::ValueTree vt);

    CiProfileDisabled (MidiGroup group, int sourceMuid, ProfileId profile,
                       int channelsDisabled = 0);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    MAKE_VALUE_MEMBER (ProfileId, profile,         {});
    MAKE_VALUE_MEMBER (int,       channelsDisabled, 0);
};

// ---------------------------------------------------------------------------

/**
 * @brief CI Profile Added Report (0x26). One-way broadcast.
 *
 * Sent when a device gains support for a new profile. Destination MUID is
 * always broadcast. Payload (after common CI header at buf[13]):
 *   [13..17] Profile ID (5 bytes)
 */
struct CiProfileAdded : public CiMessage
{
    static const inline juce::Identifier type { "CiProfileAdded" };

    explicit CiProfileAdded (const Sysex7Message& msg);
    explicit CiProfileAdded (const Event& e);
    explicit CiProfileAdded (juce::ValueTree vt);

    CiProfileAdded (MidiGroup group, int sourceMuid, ProfileId profile);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    MAKE_VALUE_MEMBER (ProfileId, profile, {});
};

// ---------------------------------------------------------------------------

/**
 * @brief CI Profile Removed Report (0x27). One-way broadcast.
 *
 * Sent when a device loses support for a profile. Destination MUID is always
 * broadcast. Payload (after common CI header at buf[13]):
 *   [13..17] Profile ID (5 bytes)
 */
struct CiProfileRemoved : public CiMessage
{
    static const inline juce::Identifier type { "CiProfileRemoved" };

    explicit CiProfileRemoved (const Sysex7Message& msg);
    explicit CiProfileRemoved (const Event& e);
    explicit CiProfileRemoved (juce::ValueTree vt);

    CiProfileRemoved (MidiGroup group, int sourceMuid, ProfileId profile);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    MAKE_VALUE_MEMBER (ProfileId, profile, {});
};

// ---------------------------------------------------------------------------

/**
 * @brief CI Profile Details Inquiry (0x28).
 *
 * Asks a responder for details about a specific profile's implementation.
 * Payload (after common CI header at buf[13]):
 *   [13..17] Profile ID (5 bytes)
 *   [18]     Inquiry Target (0x00–0x3F registered; 0x40–0x7F profile-specific)
 */
struct CiProfileDetailsInquiry : public CiMessage
{
    static const inline juce::Identifier type { "CiProfileDetailsInquiry" };

    explicit CiProfileDetailsInquiry (const Sysex7Message& msg);
    explicit CiProfileDetailsInquiry (const Event& e);
    explicit CiProfileDetailsInquiry (juce::ValueTree vt);

    CiProfileDetailsInquiry (MidiGroup group, int sourceMuid, int destMuid,
                             ProfileId profile, MidiByte inquiryTarget);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    MAKE_VALUE_MEMBER (ProfileId, profile,       {});
    MAKE_VALUE_MEMBER (int,       inquiryTarget, 0);

    bool isRegisteredTarget () const { return inquiryTarget.get () <= 0x3F; }
};

// ---------------------------------------------------------------------------

/**
 * @brief CI Reply to Profile Details Inquiry (0x29).
 *
 * Payload (after common CI header at buf[13]):
 *   [13..17] Profile ID (5 bytes)
 *   [18]     Inquiry Target
 *   [19..20] dl: Inquiry Target Data Length (LSB first)
 *   [21..]   Inquiry Target Data (dl bytes)
 *
 * Always serialized with messageFormat = 0x02 per spec.
 * targetData stores the raw response bytes; dl is derived at serialize time.
 */
struct CiProfileDetailsInquiryReply : public CiMessage
{
    static const inline juce::Identifier type { "CiProfileDetailsInquiryReply" };

    explicit CiProfileDetailsInquiryReply (const Sysex7Message& msg);
    explicit CiProfileDetailsInquiryReply (const Event& e);
    explicit CiProfileDetailsInquiryReply (juce::ValueTree vt);

    CiProfileDetailsInquiryReply (MidiGroup group, int sourceMuid, int destMuid,
                                  ProfileId profile, MidiByte inquiryTarget,
                                  Buffer::Ptr targetData);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    MAKE_VALUE_MEMBER (ProfileId,   profile,       {});
    MAKE_VALUE_MEMBER (int,         inquiryTarget, 0);
    MAKE_VALUE_MEMBER (Buffer::Ptr, targetData,    {});

    bool isRegisteredTarget () const { return inquiryTarget.get () <= 0x3F; }
};

// ---------------------------------------------------------------------------

/**
 * @brief CI Profile Specific Data (0x2F). One-way — no reply.
 *
 * Carries profile-specific data for a given Profile ID. Destination MUID may
 * be a specific MUID or broadcast. Payload (after common CI header at buf[13]):
 *   [13..17] Profile ID (5 bytes)
 *   [18..21] Data Length (4 bytes, MidiLong LSB first)
 *   [22..]   Profile Specific Data (dataLength bytes)
 */
struct CiProfileSpecificData : public CiMessage
{
    static const inline juce::Identifier type { "CiProfileSpecificData" };

    explicit CiProfileSpecificData (const Sysex7Message& msg);
    explicit CiProfileSpecificData (const Event& e);
    explicit CiProfileSpecificData (juce::ValueTree vt);

    CiProfileSpecificData (MidiGroup group, int sourceMuid, int destMuid,
                           ProfileId profile, Buffer::Ptr data);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    MAKE_VALUE_MEMBER (ProfileId,   profile,     {});
    MAKE_VALUE_MEMBER (Buffer::Ptr, profileData, {});
};
