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
 * @brief CI Discovery Inquiry (0x70).
 *
 * Wire format payload (after common CI header at buf[13]):
 *   [13..15] manufacturer SysEx ID (3 × 7-bit bytes)
 *   [16..17] device family (LSB first)
 *   [18..19] device family model number (LSB first)
 *   [20..23] software revision level (4 × 7-bit bytes)
 *   [24]     CI categories supported (bitmap)
 *   [25..28] maximum SysEx message size (MidiLong, LSB first)
 *   [29]     initiator output path ID (format 2+)
 */
struct CiDiscoveryInquiry : public CiMessage
{
    static const inline juce::Identifier type { "CiDiscoveryInquiry" };

    // Parse from an assembled Sysex7Message
    explicit CiDiscoveryInquiry (const Sysex7Message& msg);

    // Re-wrap from a stored Event (e.g. when reading back from an EventList)
    explicit CiDiscoveryInquiry (const Event& e);

    // Reconstruct from a stored ValueTree
    explicit CiDiscoveryInquiry (juce::ValueTree vt);

    // Programmatic construction (format 1 — no outputPathId)
    CiDiscoveryInquiry (MidiGroup group, int sourceMuid, ManufacturerId manufacturer,
                        DeviceFamily deviceFamily, DeviceFamilyModel deviceFamilyModel,
                        MidiByte swRev1, MidiByte swRev2, MidiByte swRev3, MidiByte swRev4,
                        int ciCategories, MidiLong maxSysexSize);

    // Serialize to a Sysex7Message. targetFormat controls which optional fields
    // are included and is written as the messageFormat byte.
    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    // Manufacturer SysEx ID — 3 independent 7-bit bytes
    MAKE_VALUE_MEMBER (int, manufacturerByte0, 0);
    MAKE_VALUE_MEMBER (int, manufacturerByte1, 0);
    MAKE_VALUE_MEMBER (int, manufacturerByte2, 0);

    // Device family — 2 bytes, LSB first
    MAKE_VALUE_MEMBER (int, deviceFamilyLsb, 0);
    MAKE_VALUE_MEMBER (int, deviceFamilyMsb, 0);

    // Device family model — 2 bytes, LSB first
    MAKE_VALUE_MEMBER (int, deviceFamilyModelLsb, 0);
    MAKE_VALUE_MEMBER (int, deviceFamilyModelMsb, 0);

    // Software revision — 4 independent 7-bit bytes
    MAKE_VALUE_MEMBER (int, revisionByte0, 0);
    MAKE_VALUE_MEMBER (int, revisionByte1, 0);
    MAKE_VALUE_MEMBER (int, revisionByte2, 0);
    MAKE_VALUE_MEMBER (int, revisionByte3, 0);

    // CI categories supported — bitmap with per-bit computed accessors
    MAKE_VALUE_MEMBER (int, ciCategoriesSupported, 0);

    MAKE_COMPUTED_VALUE_MEMBER (
        bool, protocolNegotiationSupported,
        [this] () -> bool { return (ciCategoriesSupported.get () & CiCategory::protocolNegotiation) != 0; },
        [this] (const bool& v) {
            ciCategoriesSupported = v ? (ciCategoriesSupported.get () | CiCategory::protocolNegotiation)
                                      : (ciCategoriesSupported.get () & ~CiCategory::protocolNegotiation);
        });

    MAKE_COMPUTED_VALUE_MEMBER (
        bool, profileConfigSupported,
        [this] () -> bool { return (ciCategoriesSupported.get () & CiCategory::profileConfiguration) != 0; },
        [this] (const bool& v) {
            ciCategoriesSupported = v ? (ciCategoriesSupported.get () | CiCategory::profileConfiguration)
                                      : (ciCategoriesSupported.get () & ~CiCategory::profileConfiguration);
        });

    MAKE_COMPUTED_VALUE_MEMBER (
        bool, propertyExchangeSupported,
        [this] () -> bool { return (ciCategoriesSupported.get () & CiCategory::propertyExchange) != 0; },
        [this] (const bool& v) {
            ciCategoriesSupported = v ? (ciCategoriesSupported.get () | CiCategory::propertyExchange)
                                      : (ciCategoriesSupported.get () & ~CiCategory::propertyExchange);
        });

    MAKE_COMPUTED_VALUE_MEMBER (
        bool, processInquirySupported,
        [this] () -> bool { return (ciCategoriesSupported.get () & CiCategory::processInquiry) != 0; },
        [this] (const bool& v) {
            ciCategoriesSupported = v ? (ciCategoriesSupported.get () | CiCategory::processInquiry)
                                      : (ciCategoriesSupported.get () & ~CiCategory::processInquiry);
        });

    // Maximum SysEx size — stored as combined 28-bit int (MidiLong value)
    MAKE_VALUE_MEMBER (int, maxSysexSize, 0);

    // Output path ID — format 2+; 0 if absent
    MAKE_VALUE_MEMBER (int, outputPathId, 0);
};

// ---------------------------------------------------------------------------

/**
 * @brief CI Discovery Reply (0x71).
 *
 * Same payload as Discovery Inquiry, plus:
 *   [29]  initiator output path ID (format 2+)
 *   [30]  function block (format 2+)
 */
struct CiDiscoveryReply : public CiMessage
{
    static const inline juce::Identifier type { "CiDiscoveryReply" };

    explicit CiDiscoveryReply (const Sysex7Message& msg);
    explicit CiDiscoveryReply (const Event& e);
    explicit CiDiscoveryReply (juce::ValueTree vt);

    // Programmatic construction (format 1 — no outputPathId or functionBlock)
    CiDiscoveryReply (MidiGroup group, int sourceMuid, int destMuid, ManufacturerId manufacturer,
                      DeviceFamily deviceFamily, DeviceFamilyModel deviceFamilyModel,
                      MidiByte swRev1, MidiByte swRev2, MidiByte swRev3, MidiByte swRev4,
                      int ciCategories, MidiLong maxSysexSize);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    MAKE_VALUE_MEMBER (int, manufacturerByte0, 0);
    MAKE_VALUE_MEMBER (int, manufacturerByte1, 0);
    MAKE_VALUE_MEMBER (int, manufacturerByte2, 0);

    MAKE_VALUE_MEMBER (int, deviceFamilyLsb, 0);
    MAKE_VALUE_MEMBER (int, deviceFamilyMsb, 0);

    MAKE_VALUE_MEMBER (int, deviceFamilyModelLsb, 0);
    MAKE_VALUE_MEMBER (int, deviceFamilyModelMsb, 0);

    MAKE_VALUE_MEMBER (int, revisionByte0, 0);
    MAKE_VALUE_MEMBER (int, revisionByte1, 0);
    MAKE_VALUE_MEMBER (int, revisionByte2, 0);
    MAKE_VALUE_MEMBER (int, revisionByte3, 0);

    MAKE_VALUE_MEMBER (int, ciCategoriesSupported, 0);

    MAKE_COMPUTED_VALUE_MEMBER (
        bool, protocolNegotiationSupported,
        [this] () -> bool { return (ciCategoriesSupported.get () & CiCategory::protocolNegotiation) != 0; },
        [this] (const bool& v) {
            ciCategoriesSupported = v ? (ciCategoriesSupported.get () | CiCategory::protocolNegotiation)
                                      : (ciCategoriesSupported.get () & ~CiCategory::protocolNegotiation);
        });

    MAKE_COMPUTED_VALUE_MEMBER (
        bool, profileConfigSupported,
        [this] () -> bool { return (ciCategoriesSupported.get () & CiCategory::profileConfiguration) != 0; },
        [this] (const bool& v) {
            ciCategoriesSupported = v ? (ciCategoriesSupported.get () | CiCategory::profileConfiguration)
                                      : (ciCategoriesSupported.get () & ~CiCategory::profileConfiguration);
        });

    MAKE_COMPUTED_VALUE_MEMBER (
        bool, propertyExchangeSupported,
        [this] () -> bool { return (ciCategoriesSupported.get () & CiCategory::propertyExchange) != 0; },
        [this] (const bool& v) {
            ciCategoriesSupported = v ? (ciCategoriesSupported.get () | CiCategory::propertyExchange)
                                      : (ciCategoriesSupported.get () & ~CiCategory::propertyExchange);
        });

    MAKE_COMPUTED_VALUE_MEMBER (
        bool, processInquirySupported,
        [this] () -> bool { return (ciCategoriesSupported.get () & CiCategory::processInquiry) != 0; },
        [this] (const bool& v) {
            ciCategoriesSupported = v ? (ciCategoriesSupported.get () | CiCategory::processInquiry)
                                      : (ciCategoriesSupported.get () & ~CiCategory::processInquiry);
        });

    MAKE_VALUE_MEMBER (int, maxSysexSize, 0);

    // Format 2+ fields
    MAKE_VALUE_MEMBER (int, outputPathId, 0);
    MAKE_VALUE_MEMBER (int, functionBlock, 0);
};
