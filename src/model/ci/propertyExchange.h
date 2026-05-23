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
 * @brief CI Inquiry: Property Exchange Capabilities (0x30).
 *
 * Exchanged at the start of a Property Exchange session so both devices know
 * each other's simultaneous-request limit and PE version.
 *
 * Device ID is always 0x7F (whole Function Block) per §8.2.
 *
 * Wire payload (after common CI header at buf[13]):
 *   [13]  simultaneousRequests — number of concurrent PE requests supported
 *   The following fields were added in MIDI-CI Message Version 2:
 *   [14]  majorVersion
 *   [15]  minorVersion
 */
struct CiPeCapabilitiesInquiry : public CiMessage
{
    static const inline juce::Identifier type { "CiPeCapabilitiesInquiry" };

    explicit CiPeCapabilitiesInquiry (const Sysex7Message& msg);
    explicit CiPeCapabilitiesInquiry (const Event& e);
    explicit CiPeCapabilitiesInquiry (juce::ValueTree vt);

    CiPeCapabilitiesInquiry (MidiGroup group, int sourceMuid, int destMuid,
                             int simultaneousRequests,
                             int majorVersion = 0, int minorVersion = 0);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    MAKE_VALUE_MEMBER (int, simultaneousRequests, 1);
    MAKE_VALUE_MEMBER (int, majorVersion,         0);
    MAKE_VALUE_MEMBER (int, minorVersion,         0);
};

// ---------------------------------------------------------------------------

/**
 * @brief CI Inquiry: Get Property Data (0x34).
 *
 * Asks a Responder for the value of a specific property, identified by the
 * Header Data (JSON resource/command descriptor).
 *
 * Device ID is always 0x7F (whole Function Block) per §8.2.
 *
 * Wire payload (after common CI header at buf[13]):
 *   [13]       requestId
 *   [14..15]   headerDataLength (2 bytes, LSB first)
 *   [16..16+hl-1] headerData (hl bytes)
 *   [16+hl..16+hl+1] numberOfChunks — always 0x0001 (spec-mandated)
 *   [16+hl+2..16+hl+3] chunkNumber  — always 0x0001 (spec-mandated)
 *   [16+hl+4..16+hl+5] propertyDataLength — always 0x0000 (no property data in inquiry)
 */
struct CiPeGetPropertyDataInquiry : public CiMessage
{
    static const inline juce::Identifier type { "CiPeGetPropertyDataInquiry" };

    explicit CiPeGetPropertyDataInquiry (const Sysex7Message& msg);
    explicit CiPeGetPropertyDataInquiry (const Event& e);
    explicit CiPeGetPropertyDataInquiry (juce::ValueTree vt);

    CiPeGetPropertyDataInquiry (MidiGroup group, int sourceMuid, int destMuid,
                                int requestId, Buffer::Ptr headerData);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    MAKE_VALUE_MEMBER (int,         requestId,  0);
    MAKE_VALUE_MEMBER (Buffer::Ptr, headerData, {});
};

// ---------------------------------------------------------------------------

/**
 * @brief CI Reply to Get Property Data (0x35).
 *
 * Sent by a Responder in reply to a CiPeGetPropertyDataInquiry. May be split
 * across multiple chunks; numberOfChunks == 0 means total count is unknown.
 *
 * Device ID is always 0x7F (whole Function Block) per §8.2.
 *
 * Wire payload (after common CI header at buf[13]):
 *   [13]       requestId
 *   [14..15]   headerDataLength (2 bytes, LSB first)
 *   [16..16+hl-1] headerData (hl bytes)
 *   [16+hl..16+hl+1] numberOfChunks (0x0000 = unknown)
 *   [16+hl+2..16+hl+3] chunkNumber (starts at 0x0001)
 *   [16+hl+4..16+hl+5] propertyDataLength
 *   [16+hl+6..] propertyData (propertyDataLength bytes)
 */
struct CiPeGetPropertyDataReply : public CiMessage
{
    static const inline juce::Identifier type { "CiPeGetPropertyDataReply" };

    explicit CiPeGetPropertyDataReply (const Sysex7Message& msg);
    explicit CiPeGetPropertyDataReply (const Event& e);
    explicit CiPeGetPropertyDataReply (juce::ValueTree vt);

    CiPeGetPropertyDataReply (MidiGroup group, int sourceMuid, int destMuid,
                              int requestId, Buffer::Ptr headerData,
                              int numberOfChunks, int chunkNumber,
                              Buffer::Ptr propertyData);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    MAKE_VALUE_MEMBER (int,         requestId,       0);
    MAKE_VALUE_MEMBER (Buffer::Ptr, headerData,      {});
    MAKE_VALUE_MEMBER (int,         numberOfChunks,  1);
    MAKE_VALUE_MEMBER (int,         chunkNumber,     1);
    MAKE_VALUE_MEMBER (Buffer::Ptr, propertyData,    {});
};

// ---------------------------------------------------------------------------

/**
 * @brief CI Inquiry: Set Property Data (0x36).
 *
 * Carries property data to write to a Responder. Structurally identical to
 * CiPeGetPropertyDataReply: has requestId, headerData, chunking fields, and
 * propertyData.
 *
 * Device ID is always 0x7F (whole Function Block) per §8.2.
 */
struct CiPeSetPropertyDataInquiry : public CiMessage
{
    static const inline juce::Identifier type { "CiPeSetPropertyDataInquiry" };

    explicit CiPeSetPropertyDataInquiry (const Sysex7Message& msg);
    explicit CiPeSetPropertyDataInquiry (const Event& e);
    explicit CiPeSetPropertyDataInquiry (juce::ValueTree vt);

    CiPeSetPropertyDataInquiry (MidiGroup group, int sourceMuid, int destMuid,
                                int requestId, Buffer::Ptr headerData,
                                int numberOfChunks, int chunkNumber,
                                Buffer::Ptr propertyData);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    MAKE_VALUE_MEMBER (int,         requestId,      0);
    MAKE_VALUE_MEMBER (Buffer::Ptr, headerData,     {});
    MAKE_VALUE_MEMBER (int,         numberOfChunks, 1);
    MAKE_VALUE_MEMBER (int,         chunkNumber,    1);
    MAKE_VALUE_MEMBER (Buffer::Ptr, propertyData,   {});
};

// ---------------------------------------------------------------------------

/**
 * @brief CI Reply to Set Property Data (0x37).
 *
 * Sent by a Responder after processing a Set Property Data Inquiry. Carries
 * only a header (status/result); no property data. Structurally identical to
 * CiPeGetPropertyDataInquiry: fixed numberOfChunks=1, chunkNumber=1,
 * propertyDataLength=0.
 *
 * Device ID is always 0x7F (whole Function Block) per §8.2.
 */
struct CiPeSetPropertyDataReply : public CiMessage
{
    static const inline juce::Identifier type { "CiPeSetPropertyDataReply" };

    explicit CiPeSetPropertyDataReply (const Sysex7Message& msg);
    explicit CiPeSetPropertyDataReply (const Event& e);
    explicit CiPeSetPropertyDataReply (juce::ValueTree vt);

    CiPeSetPropertyDataReply (MidiGroup group, int sourceMuid, int destMuid,
                              int requestId, Buffer::Ptr headerData);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    MAKE_VALUE_MEMBER (int,         requestId,  0);
    MAKE_VALUE_MEMBER (Buffer::Ptr, headerData, {});
};

// ---------------------------------------------------------------------------

/**
 * @brief CI Subscription (0x38).
 *
 * Sent by an Initiator to establish, update, or terminate a subscription to a
 * property. The Responder may also use this message to end the subscription.
 * Carries header (subscribe/unsubscribe command) and optional property data.
 *
 * Device ID is always 0x7F (whole Function Block) per §8.2.
 *
 * Wire layout identical to CiPeSetPropertyDataInquiry (full chunked payload).
 */
struct CiPeSubscriptionInquiry : public CiMessage
{
    static const inline juce::Identifier type { "CiPeSubscriptionInquiry" };

    explicit CiPeSubscriptionInquiry (const Sysex7Message& msg);
    explicit CiPeSubscriptionInquiry (const Event& e);
    explicit CiPeSubscriptionInquiry (juce::ValueTree vt);

    CiPeSubscriptionInquiry (MidiGroup group, int sourceMuid, int destMuid,
                             int requestId, Buffer::Ptr headerData,
                             int numberOfChunks, int chunkNumber,
                             Buffer::Ptr propertyData);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    MAKE_VALUE_MEMBER (int,         requestId,      0);
    MAKE_VALUE_MEMBER (Buffer::Ptr, headerData,     {});
    MAKE_VALUE_MEMBER (int,         numberOfChunks, 1);
    MAKE_VALUE_MEMBER (int,         chunkNumber,    1);
    MAKE_VALUE_MEMBER (Buffer::Ptr, propertyData,   {});
};

// ---------------------------------------------------------------------------

/**
 * @brief CI Reply to Subscription (0x39).
 *
 * Sent by a Device in reply to a Subscription message. Also used by the
 * Responder to send property-data updates to subscribed Initiators.
 *
 * Device ID is always 0x7F (whole Function Block) per §8.2.
 *
 * Wire layout identical to CiPeSubscriptionInquiry (full chunked payload).
 */
struct CiPeSubscriptionReply : public CiMessage
{
    static const inline juce::Identifier type { "CiPeSubscriptionReply" };

    explicit CiPeSubscriptionReply (const Sysex7Message& msg);
    explicit CiPeSubscriptionReply (const Event& e);
    explicit CiPeSubscriptionReply (juce::ValueTree vt);

    CiPeSubscriptionReply (MidiGroup group, int sourceMuid, int destMuid,
                           int requestId, Buffer::Ptr headerData,
                           int numberOfChunks, int chunkNumber,
                           Buffer::Ptr propertyData);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    MAKE_VALUE_MEMBER (int,         requestId,      0);
    MAKE_VALUE_MEMBER (Buffer::Ptr, headerData,     {});
    MAKE_VALUE_MEMBER (int,         numberOfChunks, 1);
    MAKE_VALUE_MEMBER (int,         chunkNumber,    1);
    MAKE_VALUE_MEMBER (Buffer::Ptr, propertyData,   {});
};

// ---------------------------------------------------------------------------

/**
 * @brief CI Notify (0x3F). DEPRECATED — use ACK (0x7D) / NAK (0x7F) instead.
 *
 * An informational message sent by either Initiator or Responder to report
 * error conditions or other information. Defined in Common Rules for PE [MA05].
 * Devices should not send Notify but must be able to receive it for
 * backward compatibility with legacy devices.
 *
 * Device ID is always 0x7F (whole Function Block) per §8.2.
 *
 * Wire layout identical to CiPeSubscriptionInquiry (full chunked payload).
 */
struct CiPeNotify : public CiMessage
{
    static const inline juce::Identifier type { "CiPeNotify" };

    explicit CiPeNotify (const Sysex7Message& msg);
    explicit CiPeNotify (const Event& e);
    explicit CiPeNotify (juce::ValueTree vt);

    CiPeNotify (MidiGroup group, int sourceMuid, int destMuid,
                int requestId, Buffer::Ptr headerData,
                int numberOfChunks, int chunkNumber,
                Buffer::Ptr propertyData);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    MAKE_VALUE_MEMBER (int,         requestId,      0);
    MAKE_VALUE_MEMBER (Buffer::Ptr, headerData,     {});
    MAKE_VALUE_MEMBER (int,         numberOfChunks, 1);
    MAKE_VALUE_MEMBER (int,         chunkNumber,    1);
    MAKE_VALUE_MEMBER (Buffer::Ptr, propertyData,   {});
};

// ---------------------------------------------------------------------------

/**
 * @brief CI Reply to Property Exchange Capabilities (0x31).
 *
 * Sent by a Responder in reply to a CiPeCapabilitiesInquiry. Reports the
 * Responder's own simultaneous-request limit and PE version.
 *
 * Device ID is always 0x7F (whole Function Block) per §8.2.
 * Wire layout is identical to the Inquiry (0x30).
 */
struct CiPeCapabilitiesReply : public CiMessage
{
    static const inline juce::Identifier type { "CiPeCapabilitiesReply" };

    explicit CiPeCapabilitiesReply (const Sysex7Message& msg);
    explicit CiPeCapabilitiesReply (const Event& e);
    explicit CiPeCapabilitiesReply (juce::ValueTree vt);

    CiPeCapabilitiesReply (MidiGroup group, int sourceMuid, int destMuid,
                           int simultaneousRequests,
                           int majorVersion = 0, int minorVersion = 0);

    Sysex7Message toSysex7Message (MidiNibble group, int targetFormat) const;

    MAKE_VALUE_MEMBER (int, simultaneousRequests, 1);
    MAKE_VALUE_MEMBER (int, majorVersion,         0);
    MAKE_VALUE_MEMBER (int, minorVersion,         0);
};
