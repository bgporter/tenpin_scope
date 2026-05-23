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

#include "discovery.h"

namespace
{
// Byte offsets in the Sysex7 buffer for Discovery-specific payload
constexpr size_t kMfrByte0     = 13;
constexpr size_t kMfrByte1     = 14;
constexpr size_t kMfrByte2     = 15;
constexpr size_t kFamilyLsb    = 16;
constexpr size_t kFamilyMsb    = 17;
constexpr size_t kModelLsb     = 18;
constexpr size_t kModelMsb     = 19;
constexpr size_t kRev0         = 20;
constexpr size_t kRev1         = 21;
constexpr size_t kRev2         = 22;
constexpr size_t kRev3         = 23;
constexpr size_t kCategories   = 24;
constexpr size_t kMaxSysexLsb  = 25; // 4 bytes: 25,26,27,28
constexpr size_t kOutputPathId = 29; // format 2+

// Reply adds functionBlock at:
constexpr size_t kFunctionBlock = 30; // format 2+

constexpr size_t kInquiryFormat1MinSize = 29; // bytes 0..28
constexpr size_t kInquiryFormat2MinSize = 30; // adds outputPathId
constexpr size_t kReplyFormat2MinSize   = 31; // adds functionBlock

// Collect bytes from [start, end) into a newly allocated Buffer.
Buffer::Ptr collectExtraBytes (const Buffer& src, size_t start, size_t end)
{
    if (start >= end || start >= src.size ())
        return {};
    const size_t count = std::min (end, src.size ()) - start;
    Buffer::Ptr extra  = new Buffer ();
    for (size_t i = 0; i < count; ++i)
        extra->append (src[start + i]);
    return extra;
}

void appendMidiLong (Buffer& buf, int value)
{
    MidiLong ml { value };
    buf.append (static_cast<uint8_t> (ml.getLsb ()));
    buf.append (static_cast<uint8_t> (ml.getByte2 ()));
    buf.append (static_cast<uint8_t> (ml.getByte3 ()));
    buf.append (static_cast<uint8_t> (ml.getMsb ()));
}
} // namespace

// ============================================================================
// CiDiscoveryInquiry
// ============================================================================

CiDiscoveryInquiry::CiDiscoveryInquiry (const Sysex7Message& msg)
: CiMessage { type.toString () }
{
    parseHeader (msg);

    const auto buf = msg.data.get ();
    if (!buf || buf->size () < kInquiryFormat1MinSize)
        return;

    manufacturerByte0 = static_cast<int> ((*buf)[kMfrByte0]);
    manufacturerByte1 = static_cast<int> ((*buf)[kMfrByte1]);
    manufacturerByte2 = static_cast<int> ((*buf)[kMfrByte2]);
    deviceFamilyLsb   = static_cast<int> ((*buf)[kFamilyLsb]);
    deviceFamilyMsb   = static_cast<int> ((*buf)[kFamilyMsb]);
    deviceFamilyModelLsb = static_cast<int> ((*buf)[kModelLsb]);
    deviceFamilyModelMsb = static_cast<int> ((*buf)[kModelMsb]);
    revisionByte0     = static_cast<int> ((*buf)[kRev0]);
    revisionByte1     = static_cast<int> ((*buf)[kRev1]);
    revisionByte2     = static_cast<int> ((*buf)[kRev2]);
    revisionByte3     = static_cast<int> ((*buf)[kRev3]);
    ciCategoriesSupported = static_cast<int> ((*buf)[kCategories]);
    maxSysexSize      = MidiLong { static_cast<int> ((*buf)[kMaxSysexLsb]),
                                   static_cast<int> ((*buf)[kMaxSysexLsb + 1]),
                                   static_cast<int> ((*buf)[kMaxSysexLsb + 2]),
                                   static_cast<int> ((*buf)[kMaxSysexLsb + 3]) }
                            .get ();

    if (buf->size () >= kInquiryFormat2MinSize)
        outputPathId = static_cast<int> ((*buf)[kOutputPathId]);

    // Bytes beyond what we understand for this format go into extraData
    const size_t knownEnd =
        (buf->size () >= kInquiryFormat2MinSize) ? kInquiryFormat2MinSize : kInquiryFormat1MinSize;
    extraData = collectExtraBytes (*buf, knownEnd, buf->size ());
}

CiDiscoveryInquiry::CiDiscoveryInquiry (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiDiscoveryInquiry::CiDiscoveryInquiry (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiDiscoveryInquiry::CiDiscoveryInquiry (MidiGroup theGroup, int sourceMuidValue,
                                        ManufacturerId manufacturer, DeviceFamily family,
                                        DeviceFamilyModel model, MidiByte swRev1, MidiByte swRev2,
                                        MidiByte swRev3, MidiByte swRev4, int ciCategories,
                                        MidiLong maxSysexSizeValue)
: CiMessage { type.toString () }
{
    group       = theGroup.get () - 1; // MessageBase stores 0-based
    deviceId    = CiDeviceId::functionBlock;
    messageType = CiType::discoveryInquiry;
    messageFormat = messageFormatMin;
    sourceMuid  = sourceMuidValue;
    destMuid    = broadcastMuid;

    manufacturerByte0    = manufacturer.byte0.get ();
    manufacturerByte1    = manufacturer.byte1.get ();
    manufacturerByte2    = manufacturer.byte2.get ();
    deviceFamilyLsb      = family.lsb.get ();
    deviceFamilyMsb      = family.msb.get ();
    deviceFamilyModelLsb = model.lsb.get ();
    deviceFamilyModelMsb = model.msb.get ();
    revisionByte0        = swRev1.get ();
    revisionByte1        = swRev2.get ();
    revisionByte2        = swRev3.get ();
    revisionByte3        = swRev4.get ();
    ciCategoriesSupported = ciCategories;
    maxSysexSize         = maxSysexSizeValue.get ();
}

Sysex7Message CiDiscoveryInquiry::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();

    buf->append (0x7E);
    buf->append (static_cast<uint8_t> (deviceId.get ()));
    buf->append (static_cast<uint8_t> (CiSubId::midiCi));
    buf->append (static_cast<uint8_t> (CiType::discoveryInquiry));
    buf->append (static_cast<uint8_t> (std::max (targetFormat, messageFormatMin)));
    appendMidiLong (*buf, sourceMuid.get ());
    appendMidiLong (*buf, destMuid.get ());

    buf->append (static_cast<uint8_t> (manufacturerByte0.get ()));
    buf->append (static_cast<uint8_t> (manufacturerByte1.get ()));
    buf->append (static_cast<uint8_t> (manufacturerByte2.get ()));
    buf->append (static_cast<uint8_t> (deviceFamilyLsb.get ()));
    buf->append (static_cast<uint8_t> (deviceFamilyMsb.get ()));
    buf->append (static_cast<uint8_t> (deviceFamilyModelLsb.get ()));
    buf->append (static_cast<uint8_t> (deviceFamilyModelMsb.get ()));
    buf->append (static_cast<uint8_t> (revisionByte0.get ()));
    buf->append (static_cast<uint8_t> (revisionByte1.get ()));
    buf->append (static_cast<uint8_t> (revisionByte2.get ()));
    buf->append (static_cast<uint8_t> (revisionByte3.get ()));
    buf->append (static_cast<uint8_t> (ciCategoriesSupported.get ()));
    appendMidiLong (*buf, maxSysexSize.get ());

    if (targetFormat >= 2)
        buf->append (static_cast<uint8_t> (outputPathId.get ()));

    return Sysex7Message { theGroup, buf };
}

// ============================================================================
// CiDiscoveryReply
// ============================================================================

CiDiscoveryReply::CiDiscoveryReply (const Sysex7Message& msg)
: CiMessage { type.toString () }
{
    parseHeader (msg);

    const auto buf = msg.data.get ();
    if (!buf || buf->size () < kInquiryFormat1MinSize)
        return;

    manufacturerByte0    = static_cast<int> ((*buf)[kMfrByte0]);
    manufacturerByte1    = static_cast<int> ((*buf)[kMfrByte1]);
    manufacturerByte2    = static_cast<int> ((*buf)[kMfrByte2]);
    deviceFamilyLsb      = static_cast<int> ((*buf)[kFamilyLsb]);
    deviceFamilyMsb      = static_cast<int> ((*buf)[kFamilyMsb]);
    deviceFamilyModelLsb = static_cast<int> ((*buf)[kModelLsb]);
    deviceFamilyModelMsb = static_cast<int> ((*buf)[kModelMsb]);
    revisionByte0        = static_cast<int> ((*buf)[kRev0]);
    revisionByte1        = static_cast<int> ((*buf)[kRev1]);
    revisionByte2        = static_cast<int> ((*buf)[kRev2]);
    revisionByte3        = static_cast<int> ((*buf)[kRev3]);
    ciCategoriesSupported = static_cast<int> ((*buf)[kCategories]);
    maxSysexSize         = MidiLong { static_cast<int> ((*buf)[kMaxSysexLsb]),
                                      static_cast<int> ((*buf)[kMaxSysexLsb + 1]),
                                      static_cast<int> ((*buf)[kMaxSysexLsb + 2]),
                                      static_cast<int> ((*buf)[kMaxSysexLsb + 3]) }
                               .get ();

    if (buf->size () >= kInquiryFormat2MinSize)
        outputPathId = static_cast<int> ((*buf)[kOutputPathId]);

    if (buf->size () >= kReplyFormat2MinSize)
        functionBlock = static_cast<int> ((*buf)[kFunctionBlock]);

    const size_t knownEnd = (buf->size () >= kReplyFormat2MinSize)   ? kReplyFormat2MinSize
                            : (buf->size () >= kInquiryFormat2MinSize) ? kInquiryFormat2MinSize
                                                                        : kInquiryFormat1MinSize;
    extraData = collectExtraBytes (*buf, knownEnd, buf->size ());
}

CiDiscoveryReply::CiDiscoveryReply (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiDiscoveryReply::CiDiscoveryReply (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiDiscoveryReply::CiDiscoveryReply (MidiGroup theGroup, int sourceMuidValue, int destMuidValue,
                                    ManufacturerId manufacturer, DeviceFamily family,
                                    DeviceFamilyModel model, MidiByte swRev1, MidiByte swRev2,
                                    MidiByte swRev3, MidiByte swRev4, int ciCategories,
                                    MidiLong maxSysexSizeValue)
: CiMessage { type.toString () }
{
    group         = theGroup.get () - 1;
    deviceId      = CiDeviceId::functionBlock;
    messageType   = CiType::discoveryReply;
    messageFormat = messageFormatMin;
    sourceMuid    = sourceMuidValue;
    destMuid      = destMuidValue;

    manufacturerByte0    = manufacturer.byte0.get ();
    manufacturerByte1    = manufacturer.byte1.get ();
    manufacturerByte2    = manufacturer.byte2.get ();
    deviceFamilyLsb      = family.lsb.get ();
    deviceFamilyMsb      = family.msb.get ();
    deviceFamilyModelLsb = model.lsb.get ();
    deviceFamilyModelMsb = model.msb.get ();
    revisionByte0        = swRev1.get ();
    revisionByte1        = swRev2.get ();
    revisionByte2        = swRev3.get ();
    revisionByte3        = swRev4.get ();
    ciCategoriesSupported = ciCategories;
    maxSysexSize         = maxSysexSizeValue.get ();
}

Sysex7Message CiDiscoveryReply::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();

    buf->append (0x7E);
    buf->append (static_cast<uint8_t> (deviceId.get ()));
    buf->append (static_cast<uint8_t> (CiSubId::midiCi));
    buf->append (static_cast<uint8_t> (CiType::discoveryReply));
    buf->append (static_cast<uint8_t> (std::max (targetFormat, messageFormatMin)));
    appendMidiLong (*buf, sourceMuid.get ());
    appendMidiLong (*buf, destMuid.get ());

    buf->append (static_cast<uint8_t> (manufacturerByte0.get ()));
    buf->append (static_cast<uint8_t> (manufacturerByte1.get ()));
    buf->append (static_cast<uint8_t> (manufacturerByte2.get ()));
    buf->append (static_cast<uint8_t> (deviceFamilyLsb.get ()));
    buf->append (static_cast<uint8_t> (deviceFamilyMsb.get ()));
    buf->append (static_cast<uint8_t> (deviceFamilyModelLsb.get ()));
    buf->append (static_cast<uint8_t> (deviceFamilyModelMsb.get ()));
    buf->append (static_cast<uint8_t> (revisionByte0.get ()));
    buf->append (static_cast<uint8_t> (revisionByte1.get ()));
    buf->append (static_cast<uint8_t> (revisionByte2.get ()));
    buf->append (static_cast<uint8_t> (revisionByte3.get ()));
    buf->append (static_cast<uint8_t> (ciCategoriesSupported.get ()));
    appendMidiLong (*buf, maxSysexSize.get ());

    if (targetFormat >= 2)
    {
        buf->append (static_cast<uint8_t> (outputPathId.get ()));
        buf->append (static_cast<uint8_t> (functionBlock.get ()));
    }

    return Sysex7Message { theGroup, buf };
}
