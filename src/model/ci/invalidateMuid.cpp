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

#include "invalidateMuid.h"

namespace
{
constexpr size_t kTargetMuidIdx = 13;
constexpr size_t kMinSize       = 17; // header (13) + targetMuid (4)

void appendMidiLong (Buffer& buf, int value)
{
    MidiLong ml { value };
    buf.append (static_cast<uint8_t> (ml.getLsb ()));
    buf.append (static_cast<uint8_t> (ml.getByte2 ()));
    buf.append (static_cast<uint8_t> (ml.getByte3 ()));
    buf.append (static_cast<uint8_t> (ml.getMsb ()));
}
} // namespace

CiInvalidateMuid::CiInvalidateMuid (const Sysex7Message& msg)
: CiMessage { type.toString () }
{
    parseHeader (msg);
    const auto buf = msg.data.get ();
    if (!buf || buf->size () < kMinSize)
        return;
    targetMuid = MidiLong { static_cast<int> ((*buf)[kTargetMuidIdx]),
                            static_cast<int> ((*buf)[kTargetMuidIdx + 1]),
                            static_cast<int> ((*buf)[kTargetMuidIdx + 2]),
                            static_cast<int> ((*buf)[kTargetMuidIdx + 3]) }
                     .get ();
}

CiInvalidateMuid::CiInvalidateMuid (const Event& e)
: CiMessage { type.toString (), juce::ValueTree { e } }
{
}

CiInvalidateMuid::CiInvalidateMuid (juce::ValueTree vt)
: CiMessage { type.toString (), vt }
{
}

CiInvalidateMuid::CiInvalidateMuid (MidiGroup theGroup, int sourceMuidValue, int targetMuidValue)
: CiMessage { type.toString () }
{
    group         = theGroup.get () - 1;
    deviceId      = CiDeviceId::functionBlock;
    messageType   = CiType::invalidateMuid;
    messageFormat = messageFormatMin;
    sourceMuid    = sourceMuidValue;
    destMuid      = broadcastMuid;
    targetMuid    = targetMuidValue;
}

Sysex7Message CiInvalidateMuid::toSysex7Message (MidiNibble theGroup, int targetFormat) const
{
    Buffer::Ptr buf = new Buffer ();
    buf->append (0x7E);
    buf->append (static_cast<uint8_t> (deviceId.get ()));
    buf->append (static_cast<uint8_t> (CiSubId::midiCi));
    buf->append (static_cast<uint8_t> (CiType::invalidateMuid));
    buf->append (static_cast<uint8_t> (std::max (targetFormat, messageFormatMin)));
    appendMidiLong (*buf, sourceMuid.get ());
    appendMidiLong (*buf, destMuid.get ());
    appendMidiLong (*buf, targetMuid.get ());
    return Sysex7Message { theGroup, buf };
}
