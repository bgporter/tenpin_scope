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

#include <array>

#include "model/ci/ciMessage.h"
#include "model/midiTypes.h"
#include "model/sysex/sysex7Message.h"

// ---------------------------------------------------------------------------

/**
 * @brief CI NAK (0x7F). Reply-only, no corresponding inquiry.
 *
 * Wire format payload (after common CI header at buf[13]):
 *   [13]     Original Transaction Sub-ID#2 (the type being NAKed)
 *   [14]     NAK Status Code
 *   [15]     NAK Status Data
 *   [16..20] NAK Details (5 branch-specific bytes)
 *   [21..22] Message Length ml (LSB first)
 *   [23..]   Message Text (7-bit ASCII, max ~103 bytes)
 */
struct CiNak : public CiMessage
{
    static const inline juce::Identifier type { "CiNak" };

    static constexpr size_t kDetailBytes       = 5;
    static constexpr size_t kMaxMessageTextLen = 103;

    explicit CiNak (const Sysex7Message& msg);
    explicit CiNak (const Event& e);
    explicit CiNak (juce::ValueTree vt);

    // Programmatic construction.
    CiNak (MidiGroup group, int sourceMuid, int destMuid,
           MidiByte originalSubId, MidiByte statusCode, MidiByte statusData,
           const std::array<uint8_t, kDetailBytes>& details,
           const juce::String& messageText);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    // The Sub-ID#2 of the transaction this NAK is responding to.
    MAKE_VALUE_MEMBER (int, originalSubId, 0);

    MAKE_VALUE_MEMBER (int, statusCode, 0);
    MAKE_VALUE_MEMBER (int, statusData, 0);

    // Branch-specific detail bytes (5 fixed).
    MAKE_VALUE_MEMBER (int, nakDetail0, 0);
    MAKE_VALUE_MEMBER (int, nakDetail1, 0);
    MAKE_VALUE_MEMBER (int, nakDetail2, 0);
    MAKE_VALUE_MEMBER (int, nakDetail3, 0);
    MAKE_VALUE_MEMBER (int, nakDetail4, 0);

    // Human-readable message text (7-bit ASCII; \u escapes left as-is).
    MAKE_VALUE_MEMBER (juce::String, messageText, "");
};

// ---------------------------------------------------------------------------

/**
 * @brief CI ACK (0x7D). Reply-only, no corresponding inquiry.
 *
 * Wire format payload (after common CI header at buf[13]):
 *   [13]     Original Transaction Sub-ID#2 (the type being ACKed)
 *   [14]     ACK Status Code
 *   [15]     ACK Status Data
 *   [16..20] ACK Details (5 branch-specific bytes)
 *   [21..22] Message Length ml (LSB first)
 *   [23..]   Message Text (7-bit ASCII, max ~103 bytes)
 */
struct CiAck : public CiMessage
{
    static const inline juce::Identifier type { "CiAck" };

    static constexpr size_t kDetailBytes       = 5;
    static constexpr size_t kMaxMessageTextLen = 103;

    explicit CiAck (const Sysex7Message& msg);
    explicit CiAck (const Event& e);
    explicit CiAck (juce::ValueTree vt);

    // Programmatic construction.
    CiAck (MidiGroup group, int sourceMuid, int destMuid,
           MidiByte originalSubId, MidiByte statusCode, MidiByte statusData,
           const std::array<uint8_t, kDetailBytes>& details,
           const juce::String& messageText);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    // The Sub-ID#2 of the transaction this ACK is responding to.
    MAKE_VALUE_MEMBER (int, originalSubId, 0);

    MAKE_VALUE_MEMBER (int, statusCode, 0);
    MAKE_VALUE_MEMBER (int, statusData, 0);

    // Branch-specific detail bytes (5 fixed).
    MAKE_VALUE_MEMBER (int, ackDetail0, 0);
    MAKE_VALUE_MEMBER (int, ackDetail1, 0);
    MAKE_VALUE_MEMBER (int, ackDetail2, 0);
    MAKE_VALUE_MEMBER (int, ackDetail3, 0);
    MAKE_VALUE_MEMBER (int, ackDetail4, 0);

    // Human-readable message text (7-bit ASCII; \u escapes left as-is).
    MAKE_VALUE_MEMBER (juce::String, messageText, "");
};
