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

    static constexpr size_t kProfileSize = 5;

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

    // Packed 5-byte-per-profile raw storage.
    MAKE_VALUE_MEMBER (Buffer::Ptr, enabledProfiles,  {});
    MAKE_VALUE_MEMBER (Buffer::Ptr, disabledProfiles, {});
};
