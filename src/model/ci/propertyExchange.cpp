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

#include "propertyExchange.h"

namespace
{
// PE capabilities offsets
constexpr size_t kSimultaneousRequestsIdx = 13;
constexpr size_t kMajorVersionIdx         = 14;
constexpr size_t kMinorVersionIdx         = 15;
constexpr size_t kV1Size                  = 14; // header (13) + simultaneousRequests
constexpr size_t kV2Size                  = 16; // + majorVersion + minorVersion

void appendMidiLong (Buffer& buf, int value)
{
    MidiLong ml { value };
    buf.append (static_cast<uint8_t> (ml.getLsb ()));
    buf.append (static_cast<uint8_t> (ml.getByte2 ()));
    buf.append (static_cast<uint8_t> (ml.getByte3 ()));
    buf.append (static_cast<uint8_t> (ml.getMsb ()));
}

void appendCommonHeader (Buffer& buf, int msgType, int msgFmt, int src, int dst)
{
    buf.append (0x7E);
    buf.append (static_cast<uint8_t> (CiDeviceId::functionBlock)); // always 0x7F for PE
    buf.append (static_cast<uint8_t> (CiSubId::midiCi));
    buf.append (static_cast<uint8_t> (msgType));
    buf.append (static_cast<uint8_t> (std::max (msgFmt, messageFormatMin)));
    appendMidiLong (buf, src);
    appendMidiLong (buf, dst);
}

void parsePeCapabilities (const Sysex7Message& msg, int& simReqs, int& major, int& minor,
                          Buffer::Ptr& extra)
{
    if (auto buf { msg.data.get () }; buf != nullptr)
    {
        if (buf->size () > kSimultaneousRequestsIdx)
            simReqs = static_cast<int> ((*buf)[kSimultaneousRequestsIdx]);
        if (buf->size () >= kV2Size)
        {
            major = static_cast<int> ((*buf)[kMajorVersionIdx]);
            minor = static_cast<int> ((*buf)[kMinorVersionIdx]);
        }
        const size_t extraStart = (buf->size () >= kV2Size) ? kV2Size : kV1Size;
        if (buf->size () > extraStart)
        {
            Buffer::Ptr e = new Buffer ();
            for (size_t i = extraStart; i < buf->size (); ++i)
                e->append ((*buf)[i]);
            extra = e;
        }
    }
}

Sysex7Message buildPeCapabilities (MidiNibble group, int msgType, int targetFormat,
                                   int src, int dst, int simReqs, int major, int minor)
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, msgType, targetFormat, src, dst);
    buf->append (static_cast<uint8_t> (simReqs));
    if (targetFormat >= 2)
    {
        buf->append (static_cast<uint8_t> (major));
        buf->append (static_cast<uint8_t> (minor));
    }
    return Sysex7Message { group, buf };
}
// PE get/set property offsets
constexpr size_t kPeRequestIdIdx   = 13;
constexpr size_t kPeHeaderLenIdx   = 14; // 2 bytes LSB first
constexpr size_t kPeHeaderDataIdx  = 16; // variable; remainder computed from length

void appendWord (Buffer& buf, int value)
{
    buf.append (static_cast<uint8_t> (value & 0x7F));
    buf.append (static_cast<uint8_t> ((value >> 7) & 0x7F));
}

int readWord (const Buffer& buf, size_t offset)
{
    if (offset + 1 >= buf.size ())
        return 0;
    return static_cast<int> (buf[offset]) | (static_cast<int> (buf[offset + 1]) << 7);
}

Buffer::Ptr readBytes (const Buffer& buf, size_t offset, size_t count)
{
    if (count == 0)
        return {};
    Buffer::Ptr out = new Buffer ();
    for (size_t i = 0; i < count && offset + i < buf.size (); ++i)
        out->append (buf[offset + i]);
    return out;
}
} // namespace

// ---------------------------------------------------------------------------
// CiPeCapabilitiesInquiry (0x30)
// ---------------------------------------------------------------------------

CiPeCapabilitiesInquiry::CiPeCapabilitiesInquiry (const Sysex7Message& msg)
: CiMessage (type.toString ())
{
    parseHeader (msg);
    int simReqs { 0 }, major { 0 }, minor { 0 };
    Buffer::Ptr extra;
    parsePeCapabilities (msg, simReqs, major, minor, extra);
    simultaneousRequests = simReqs;
    majorVersion         = major;
    minorVersion         = minor;
    if (extra != nullptr)
        extraData = extra;
}

CiPeCapabilitiesInquiry::CiPeCapabilitiesInquiry (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiPeCapabilitiesInquiry::CiPeCapabilitiesInquiry (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiPeCapabilitiesInquiry::CiPeCapabilitiesInquiry (MidiGroup group, int sourceMuid, int destMuid,
                                                   int simReqs, int major, int minor)
: CiMessage (type.toString ())
{
    deviceId             = CiDeviceId::functionBlock;
    messageType          = CiType::peCapabilitiesInquiry;
    this->group          = group.get ();
    this->sourceMuid     = sourceMuid;
    this->destMuid       = destMuid;
    simultaneousRequests = simReqs;
    majorVersion         = major;
    minorVersion         = minor;
}

Sysex7Message CiPeCapabilitiesInquiry::toSysex7Message (MidiNibble group, int targetFormat) const
{
    return buildPeCapabilities (group, CiType::peCapabilitiesInquiry, targetFormat,
                                sourceMuid.get (), destMuid.get (),
                                simultaneousRequests.get (),
                                majorVersion.get (), minorVersion.get ());
}

// ---------------------------------------------------------------------------
// CiPeCapabilitiesReply (0x31)
// ---------------------------------------------------------------------------

CiPeCapabilitiesReply::CiPeCapabilitiesReply (const Sysex7Message& msg)
: CiMessage (type.toString ())
{
    parseHeader (msg);
    int simReqs { 0 }, major { 0 }, minor { 0 };
    Buffer::Ptr extra;
    parsePeCapabilities (msg, simReqs, major, minor, extra);
    simultaneousRequests = simReqs;
    majorVersion         = major;
    minorVersion         = minor;
    if (extra != nullptr)
        extraData = extra;
}

CiPeCapabilitiesReply::CiPeCapabilitiesReply (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiPeCapabilitiesReply::CiPeCapabilitiesReply (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiPeCapabilitiesReply::CiPeCapabilitiesReply (MidiGroup group, int sourceMuid, int destMuid,
                                               int simReqs, int major, int minor)
: CiMessage (type.toString ())
{
    deviceId             = CiDeviceId::functionBlock;
    messageType          = CiType::peCapabilitiesReply;
    this->group          = group.get ();
    this->sourceMuid     = sourceMuid;
    this->destMuid       = destMuid;
    simultaneousRequests = simReqs;
    majorVersion         = major;
    minorVersion         = minor;
}

Sysex7Message CiPeCapabilitiesReply::toSysex7Message (MidiNibble group, int targetFormat) const
{
    return buildPeCapabilities (group, CiType::peCapabilitiesReply, targetFormat,
                                sourceMuid.get (), destMuid.get (),
                                simultaneousRequests.get (),
                                majorVersion.get (), minorVersion.get ());
}

// ---------------------------------------------------------------------------
// CiPeGetPropertyDataInquiry (0x34)
// ---------------------------------------------------------------------------

CiPeGetPropertyDataInquiry::CiPeGetPropertyDataInquiry (const Sysex7Message& msg)
: CiMessage (type.toString ())
{
    parseHeader (msg);
    if (auto buf { msg.data.get () }; buf != nullptr && buf->size () > kPeRequestIdIdx)
    {
        requestId = static_cast<int> ((*buf)[kPeRequestIdIdx]);
        const int hl = readWord (*buf, kPeHeaderLenIdx);
        headerData   = readBytes (*buf, kPeHeaderDataIdx, static_cast<size_t> (hl));
        // numberOfChunks, chunkNumber, propertyDataLength are spec-fixed (1,1,0) — not stored
    }
}

CiPeGetPropertyDataInquiry::CiPeGetPropertyDataInquiry (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiPeGetPropertyDataInquiry::CiPeGetPropertyDataInquiry (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiPeGetPropertyDataInquiry::CiPeGetPropertyDataInquiry (MidiGroup group, int sourceMuid,
                                                         int destMuid, int reqId,
                                                         Buffer::Ptr hdrData)
: CiMessage (type.toString ())
{
    deviceId         = CiDeviceId::functionBlock;
    messageType      = CiType::peGetPropertyDataInquiry;
    this->group      = group.get ();
    this->sourceMuid = sourceMuid;
    this->destMuid   = destMuid;
    requestId        = reqId;
    headerData       = hdrData;
}

Sysex7Message CiPeGetPropertyDataInquiry::toSysex7Message (MidiNibble group,
                                                             int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, CiType::peGetPropertyDataInquiry, targetFormat,
                        sourceMuid.get (), destMuid.get ());
    buf->append (static_cast<uint8_t> (requestId.get ()));

    const auto hdr { headerData.get () };
    const int hl = (hdr != nullptr) ? static_cast<int> (hdr->size ()) : 0;
    appendWord (*buf, hl);
    if (hdr != nullptr)
        for (size_t i = 0; i < hdr->size (); ++i)
            buf->append ((*hdr)[i]);

    appendWord (*buf, 1); // numberOfChunks = 1
    appendWord (*buf, 1); // chunkNumber    = 1
    appendWord (*buf, 0); // propertyDataLength = 0 (no data in inquiry)

    return Sysex7Message { group, buf };
}

// ---------------------------------------------------------------------------
// CiPeGetPropertyDataReply (0x35)
// ---------------------------------------------------------------------------

CiPeGetPropertyDataReply::CiPeGetPropertyDataReply (const Sysex7Message& msg)
: CiMessage (type.toString ())
{
    parseHeader (msg);
    if (auto buf { msg.data.get () }; buf != nullptr && buf->size () > kPeRequestIdIdx)
    {
        requestId = static_cast<int> ((*buf)[kPeRequestIdIdx]);
        const int hl  = readWord (*buf, kPeHeaderLenIdx);
        headerData    = readBytes (*buf, kPeHeaderDataIdx, static_cast<size_t> (hl));
        const size_t postHdr = kPeHeaderDataIdx + static_cast<size_t> (hl);
        numberOfChunks = readWord (*buf, postHdr);
        chunkNumber    = readWord (*buf, postHdr + 2);
        const int dl   = readWord (*buf, postHdr + 4);
        propertyData   = readBytes (*buf, postHdr + 6, static_cast<size_t> (dl));
    }
}

CiPeGetPropertyDataReply::CiPeGetPropertyDataReply (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiPeGetPropertyDataReply::CiPeGetPropertyDataReply (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiPeGetPropertyDataReply::CiPeGetPropertyDataReply (MidiGroup group, int sourceMuid,
                                                     int destMuid, int reqId,
                                                     Buffer::Ptr hdrData,
                                                     int numChunks, int chunkNum,
                                                     Buffer::Ptr propData)
: CiMessage (type.toString ())
{
    deviceId         = CiDeviceId::functionBlock;
    messageType      = CiType::peGetPropertyDataReply;
    this->group      = group.get ();
    this->sourceMuid = sourceMuid;
    this->destMuid   = destMuid;
    requestId        = reqId;
    headerData       = hdrData;
    numberOfChunks   = numChunks;
    chunkNumber      = chunkNum;
    propertyData     = propData;
}

Sysex7Message CiPeGetPropertyDataReply::toSysex7Message (MidiNibble group,
                                                           int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, CiType::peGetPropertyDataReply, targetFormat,
                        sourceMuid.get (), destMuid.get ());
    buf->append (static_cast<uint8_t> (requestId.get ()));

    const auto hdr { headerData.get () };
    const int hl = (hdr != nullptr) ? static_cast<int> (hdr->size ()) : 0;
    appendWord (*buf, hl);
    if (hdr != nullptr)
        for (size_t i = 0; i < hdr->size (); ++i)
            buf->append ((*hdr)[i]);

    appendWord (*buf, numberOfChunks.get ());
    appendWord (*buf, chunkNumber.get ());

    const auto prop { propertyData.get () };
    const int dl = (prop != nullptr) ? static_cast<int> (prop->size ()) : 0;
    appendWord (*buf, dl);
    if (prop != nullptr)
        for (size_t i = 0; i < prop->size (); ++i)
            buf->append ((*prop)[i]);

    return Sysex7Message { group, buf };
}

// ---------------------------------------------------------------------------
// CiPeSetPropertyDataInquiry (0x36) — has property data; mirrors GetReply layout
// ---------------------------------------------------------------------------

CiPeSetPropertyDataInquiry::CiPeSetPropertyDataInquiry (const Sysex7Message& msg)
: CiMessage (type.toString ())
{
    parseHeader (msg);
    if (auto buf { msg.data.get () }; buf != nullptr && buf->size () > kPeRequestIdIdx)
    {
        requestId = static_cast<int> ((*buf)[kPeRequestIdIdx]);
        const int hl  = readWord (*buf, kPeHeaderLenIdx);
        headerData    = readBytes (*buf, kPeHeaderDataIdx, static_cast<size_t> (hl));
        const size_t postHdr = kPeHeaderDataIdx + static_cast<size_t> (hl);
        numberOfChunks = readWord (*buf, postHdr);
        chunkNumber    = readWord (*buf, postHdr + 2);
        const int dl   = readWord (*buf, postHdr + 4);
        propertyData   = readBytes (*buf, postHdr + 6, static_cast<size_t> (dl));
    }
}

CiPeSetPropertyDataInquiry::CiPeSetPropertyDataInquiry (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiPeSetPropertyDataInquiry::CiPeSetPropertyDataInquiry (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiPeSetPropertyDataInquiry::CiPeSetPropertyDataInquiry (MidiGroup group, int sourceMuid,
                                                         int destMuid, int reqId,
                                                         Buffer::Ptr hdrData,
                                                         int numChunks, int chunkNum,
                                                         Buffer::Ptr propData)
: CiMessage (type.toString ())
{
    deviceId         = CiDeviceId::functionBlock;
    messageType      = CiType::peSetPropertyDataInquiry;
    this->group      = group.get ();
    this->sourceMuid = sourceMuid;
    this->destMuid   = destMuid;
    requestId        = reqId;
    headerData       = hdrData;
    numberOfChunks   = numChunks;
    chunkNumber      = chunkNum;
    propertyData     = propData;
}

Sysex7Message CiPeSetPropertyDataInquiry::toSysex7Message (MidiNibble group,
                                                             int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, CiType::peSetPropertyDataInquiry, targetFormat,
                        sourceMuid.get (), destMuid.get ());
    buf->append (static_cast<uint8_t> (requestId.get ()));

    const auto hdr { headerData.get () };
    const int hl = (hdr != nullptr) ? static_cast<int> (hdr->size ()) : 0;
    appendWord (*buf, hl);
    if (hdr != nullptr)
        for (size_t i = 0; i < hdr->size (); ++i)
            buf->append ((*hdr)[i]);

    appendWord (*buf, numberOfChunks.get ());
    appendWord (*buf, chunkNumber.get ());

    const auto prop { propertyData.get () };
    const int dl = (prop != nullptr) ? static_cast<int> (prop->size ()) : 0;
    appendWord (*buf, dl);
    if (prop != nullptr)
        for (size_t i = 0; i < prop->size (); ++i)
            buf->append ((*prop)[i]);

    return Sysex7Message { group, buf };
}

// ---------------------------------------------------------------------------
// CiPeSetPropertyDataReply (0x37) — header only; mirrors GetInquiry layout
// ---------------------------------------------------------------------------

CiPeSetPropertyDataReply::CiPeSetPropertyDataReply (const Sysex7Message& msg)
: CiMessage (type.toString ())
{
    parseHeader (msg);
    if (auto buf { msg.data.get () }; buf != nullptr && buf->size () > kPeRequestIdIdx)
    {
        requestId = static_cast<int> ((*buf)[kPeRequestIdIdx]);
        const int hl = readWord (*buf, kPeHeaderLenIdx);
        headerData   = readBytes (*buf, kPeHeaderDataIdx, static_cast<size_t> (hl));
    }
}

CiPeSetPropertyDataReply::CiPeSetPropertyDataReply (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiPeSetPropertyDataReply::CiPeSetPropertyDataReply (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiPeSetPropertyDataReply::CiPeSetPropertyDataReply (MidiGroup group, int sourceMuid,
                                                     int destMuid, int reqId,
                                                     Buffer::Ptr hdrData)
: CiMessage (type.toString ())
{
    deviceId         = CiDeviceId::functionBlock;
    messageType      = CiType::peSetPropertyDataReply;
    this->group      = group.get ();
    this->sourceMuid = sourceMuid;
    this->destMuid   = destMuid;
    requestId        = reqId;
    headerData       = hdrData;
}

Sysex7Message CiPeSetPropertyDataReply::toSysex7Message (MidiNibble group,
                                                           int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, CiType::peSetPropertyDataReply, targetFormat,
                        sourceMuid.get (), destMuid.get ());
    buf->append (static_cast<uint8_t> (requestId.get ()));

    const auto hdr { headerData.get () };
    const int hl = (hdr != nullptr) ? static_cast<int> (hdr->size ()) : 0;
    appendWord (*buf, hl);
    if (hdr != nullptr)
        for (size_t i = 0; i < hdr->size (); ++i)
            buf->append ((*hdr)[i]);

    appendWord (*buf, 1); // numberOfChunks = 1
    appendWord (*buf, 1); // chunkNumber    = 1
    appendWord (*buf, 0); // propertyDataLength = 0

    return Sysex7Message { group, buf };
}

// ---------------------------------------------------------------------------
// Shared helper for messages with full chunked payload (Subscription pair)
// ---------------------------------------------------------------------------

namespace
{
Sysex7Message buildPeChunkedPayload (MidiNibble group, int msgType, int targetFormat,
                                     int src, int dst, int reqId,
                                     Buffer::Ptr hdrData, int numChunks, int chunkNum,
                                     Buffer::Ptr propData)
{
    Buffer::Ptr buf = new Buffer ();
    appendCommonHeader (*buf, msgType, targetFormat, src, dst);
    buf->append (static_cast<uint8_t> (reqId));

    const auto hdr { hdrData.get () };
    const int hl = (hdr != nullptr) ? static_cast<int> (hdr->size ()) : 0;
    appendWord (*buf, hl);
    if (hdr != nullptr)
        for (size_t i = 0; i < hdr->size (); ++i)
            buf->append ((*hdr)[i]);

    appendWord (*buf, numChunks);
    appendWord (*buf, chunkNum);

    const auto prop { propData.get () };
    const int dl = (prop != nullptr) ? static_cast<int> (prop->size ()) : 0;
    appendWord (*buf, dl);
    if (prop != nullptr)
        for (size_t i = 0; i < prop->size (); ++i)
            buf->append ((*prop)[i]);

    return Sysex7Message { group, buf };
}

void parseChunkedPayload (const Sysex7Message& msg, int& reqId, Buffer::Ptr& hdrData,
                          int& numChunks, int& chunkNum, Buffer::Ptr& propData)
{
    if (auto buf { msg.data.get () }; buf != nullptr && buf->size () > kPeRequestIdIdx)
    {
        reqId = static_cast<int> ((*buf)[kPeRequestIdIdx]);
        const int hl  = readWord (*buf, kPeHeaderLenIdx);
        hdrData       = readBytes (*buf, kPeHeaderDataIdx, static_cast<size_t> (hl));
        const size_t postHdr = kPeHeaderDataIdx + static_cast<size_t> (hl);
        numChunks = readWord (*buf, postHdr);
        chunkNum  = readWord (*buf, postHdr + 2);
        const int dl = readWord (*buf, postHdr + 4);
        propData  = readBytes (*buf, postHdr + 6, static_cast<size_t> (dl));
    }
}
} // namespace

// ---------------------------------------------------------------------------
// CiPeSubscriptionInquiry (0x38)
// ---------------------------------------------------------------------------

CiPeSubscriptionInquiry::CiPeSubscriptionInquiry (const Sysex7Message& msg)
: CiMessage (type.toString ())
{
    parseHeader (msg);
    int reqId { 0 }, nc { 1 }, cn { 1 };
    Buffer::Ptr hdr, prop;
    parseChunkedPayload (msg, reqId, hdr, nc, cn, prop);
    requestId      = reqId;
    headerData     = hdr;
    numberOfChunks = nc;
    chunkNumber    = cn;
    propertyData   = prop;
}

CiPeSubscriptionInquiry::CiPeSubscriptionInquiry (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiPeSubscriptionInquiry::CiPeSubscriptionInquiry (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiPeSubscriptionInquiry::CiPeSubscriptionInquiry (MidiGroup group, int sourceMuid,
                                                   int destMuid, int reqId,
                                                   Buffer::Ptr hdrData,
                                                   int numChunks, int chunkNum,
                                                   Buffer::Ptr propData)
: CiMessage (type.toString ())
{
    deviceId         = CiDeviceId::functionBlock;
    messageType      = CiType::peSubscriptionInquiry;
    this->group      = group.get ();
    this->sourceMuid = sourceMuid;
    this->destMuid   = destMuid;
    requestId        = reqId;
    headerData       = hdrData;
    numberOfChunks   = numChunks;
    chunkNumber      = chunkNum;
    propertyData     = propData;
}

Sysex7Message CiPeSubscriptionInquiry::toSysex7Message (MidiNibble group,
                                                          int targetFormat) const
{
    return buildPeChunkedPayload (group, CiType::peSubscriptionInquiry, targetFormat,
                                  sourceMuid.get (), destMuid.get (),
                                  requestId.get (), headerData.get (),
                                  numberOfChunks.get (), chunkNumber.get (),
                                  propertyData.get ());
}

// ---------------------------------------------------------------------------
// CiPeSubscriptionReply (0x39)
// ---------------------------------------------------------------------------

CiPeSubscriptionReply::CiPeSubscriptionReply (const Sysex7Message& msg)
: CiMessage (type.toString ())
{
    parseHeader (msg);
    int reqId { 0 }, nc { 1 }, cn { 1 };
    Buffer::Ptr hdr, prop;
    parseChunkedPayload (msg, reqId, hdr, nc, cn, prop);
    requestId      = reqId;
    headerData     = hdr;
    numberOfChunks = nc;
    chunkNumber    = cn;
    propertyData   = prop;
}

CiPeSubscriptionReply::CiPeSubscriptionReply (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiPeSubscriptionReply::CiPeSubscriptionReply (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiPeSubscriptionReply::CiPeSubscriptionReply (MidiGroup group, int sourceMuid,
                                               int destMuid, int reqId,
                                               Buffer::Ptr hdrData,
                                               int numChunks, int chunkNum,
                                               Buffer::Ptr propData)
: CiMessage (type.toString ())
{
    deviceId         = CiDeviceId::functionBlock;
    messageType      = CiType::peSubscriptionReply;
    this->group      = group.get ();
    this->sourceMuid = sourceMuid;
    this->destMuid   = destMuid;
    requestId        = reqId;
    headerData       = hdrData;
    numberOfChunks   = numChunks;
    chunkNumber      = chunkNum;
    propertyData     = propData;
}

Sysex7Message CiPeSubscriptionReply::toSysex7Message (MidiNibble group,
                                                        int targetFormat) const
{
    return buildPeChunkedPayload (group, CiType::peSubscriptionReply, targetFormat,
                                  sourceMuid.get (), destMuid.get (),
                                  requestId.get (), headerData.get (),
                                  numberOfChunks.get (), chunkNumber.get (),
                                  propertyData.get ());
}

// ---------------------------------------------------------------------------
// CiPeNotify (0x3F) — deprecated; use ACK/NAK for new devices
// ---------------------------------------------------------------------------

CiPeNotify::CiPeNotify (const Sysex7Message& msg)
: CiMessage (type.toString ())
{
    parseHeader (msg);
    int reqId { 0 }, nc { 1 }, cn { 1 };
    Buffer::Ptr hdr, prop;
    parseChunkedPayload (msg, reqId, hdr, nc, cn, prop);
    requestId      = reqId;
    headerData     = hdr;
    numberOfChunks = nc;
    chunkNumber    = cn;
    propertyData   = prop;
}

CiPeNotify::CiPeNotify (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiPeNotify::CiPeNotify (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiPeNotify::CiPeNotify (MidiGroup group, int sourceMuid, int destMuid,
                         int reqId, Buffer::Ptr hdrData,
                         int numChunks, int chunkNum, Buffer::Ptr propData)
: CiMessage (type.toString ())
{
    deviceId         = CiDeviceId::functionBlock;
    messageType      = CiType::peNotify;
    this->group      = group.get ();
    this->sourceMuid = sourceMuid;
    this->destMuid   = destMuid;
    requestId        = reqId;
    headerData       = hdrData;
    numberOfChunks   = numChunks;
    chunkNumber      = chunkNum;
    propertyData     = propData;
}

Sysex7Message CiPeNotify::toSysex7Message (MidiNibble group, int targetFormat) const
{
    return buildPeChunkedPayload (group, CiType::peNotify, targetFormat,
                                  sourceMuid.get (), destMuid.get (),
                                  requestId.get (), headerData.get (),
                                  numberOfChunks.get (), chunkNumber.get (),
                                  propertyData.get ());
}
