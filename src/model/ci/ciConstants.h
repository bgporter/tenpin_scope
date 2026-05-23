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

#include "model/midiTypes.h"

namespace CiDeviceId
{
constexpr int group         = 0x7E;
constexpr int functionBlock = 0x7F;
} // namespace CiDeviceId

namespace CiSubId
{
constexpr int midiCi = 0x0D;
} // namespace CiSubId

namespace CiType
{
constexpr int discoveryInquiry = 0x70;
constexpr int discoveryReply   = 0x71;
} // namespace CiType

namespace CiCategory
{
constexpr int protocolNegotiation  = 0x02;
constexpr int profileConfiguration = 0x04;
constexpr int propertyExchange     = 0x08;
constexpr int processInquiry       = 0x10;
} // namespace CiCategory

constexpr int broadcastMuid    = MidiLong::maxValue; // 0x0FFFFFFF
constexpr int messageFormatMin = 0x01;               // format 0x00 is disallowed
