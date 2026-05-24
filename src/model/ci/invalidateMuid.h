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
 * @brief CI Invalidate MUID (0x7E). One-way — no reply.
 *
 * Destination MUID is always broadcast (0x0FFFFFFF).
 *
 * Wire format payload (after common CI header at buf[13]):
 *   [13..16] Target MUID (4 bytes, 7-bit each, LSB first)
 */
struct CiInvalidateMuid : public CiMessage
{
    static const inline juce::Identifier type { "CiInvalidateMuid" };

    explicit CiInvalidateMuid (const Sysex7Message& msg);
    explicit CiInvalidateMuid (const Event& e);
    explicit CiInvalidateMuid (juce::ValueTree vt);

    // Programmatic construction. destMuid is always broadcast.
    CiInvalidateMuid (MidiGroup group, int sourceMuid, int targetMuidValue);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    // The MUID being declared invalid.
    MAKE_VALUE_MEMBER (int, targetMuid, 0);
};
