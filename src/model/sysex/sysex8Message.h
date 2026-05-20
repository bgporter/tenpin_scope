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

#include <JuceHeader.h>

#include "model/message.h"
#include "model/midiTypes.h"
#include "utility/buffer.h"

struct Sysex8Message : public MessageBase
{
    static const inline juce::Identifier type { "MsgSysex8" };

    // Re-wrap an existing Event whose ValueTree is already a MsgSysex8
    Sysex8Message (const Event& e);

    // Reconstruct directly from a stored ValueTree (e.g. when reading back from an EventList)
    Sysex8Message (juce::ValueTree vt);

    // Build a fresh message from accumulated buffer data
    Sysex8Message (MidiNibble group, int streamId, Buffer::Ptr data);

    MAKE_VALUE_MEMBER (int,         streamId, {});
    MAKE_VALUE_MEMBER (Buffer::Ptr, data,     {});

private:
    using MessageBase::channel;
    using MessageBase::channelId;
    using MessageBase::userChannel;
    using MessageBase::userChannelId;
};
