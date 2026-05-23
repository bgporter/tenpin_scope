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
 * @brief CI Endpoint Information Inquiry (0x72).
 *
 * Wire format payload (after common CI header at buf[13]):
 *   [13] status  (0x00 = request Product Instance ID)
 */
struct CiEndpointInquiry : public CiMessage
{
    static const inline juce::Identifier type { "CiEndpointInquiry" };

    explicit CiEndpointInquiry (const Sysex7Message& msg);
    explicit CiEndpointInquiry (const Event& e);
    explicit CiEndpointInquiry (juce::ValueTree vt);

    // Programmatic construction
    CiEndpointInquiry (MidiGroup group, int sourceMuid, int destMuid, MidiByte status);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    MAKE_VALUE_MEMBER (int, status, 0);
};

// ---------------------------------------------------------------------------

/**
 * @brief CI Endpoint Information Reply (0x73).
 *
 * Wire format payload (after common CI header at buf[13]):
 *   [13]     status
 *   [14..15] length of information data (LSB first)
 *   [16..]   information data (ASCII, 32–126, max 42 bytes)
 */
struct CiEndpointReply : public CiMessage
{
    static const inline juce::Identifier type { "CiEndpointReply" };

    static constexpr int maxProductInstanceIdLength = 42;

    explicit CiEndpointReply (const Sysex7Message& msg);
    explicit CiEndpointReply (const Event& e);
    explicit CiEndpointReply (juce::ValueTree vt);

    // Programmatic construction. Logs an error and filters the string if it
    // contains characters outside 32–126 or exceeds maxProductInstanceIdLength.
    CiEndpointReply (MidiGroup group, int sourceMuid, int destMuid, MidiByte status,
                     const juce::String& productId);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    MAKE_VALUE_MEMBER (int,          status,            0);
    MAKE_VALUE_MEMBER (juce::String, productInstanceId, "");
};
