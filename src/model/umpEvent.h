/*
 MIT License

 Copyright (c) 2026 Brett g Porter

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to so, subject to the following conditions:

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

#include "bitField.h"
#include "utility/variantConverters.h"

struct UmpEvent : public cello::Object
{
    static const inline juce::Identifier type { "UmpEvent" };
    UmpEvent (const juce::ump::View& view, double timestamp, int endpointIndex)
    : cello::Object { type.toString (), nullptr }
    {
        // is this too clever?
        switch (view.size ())
        {
            case 4:
                data3 = view[3];
                [[fallthrough]];
            case 3:
                data2 = view[2];
                [[fallthrough]];
            case 2:
                data1 = view[1];
                [[fallthrough]];
            default:
                data0 = view[0];
                break;
        }
        this->timestamp     = timestamp;
        this->endpointIndex = endpointIndex;
    }

    UmpEvent (juce::ValueTree valueTree)
    : cello::Object { type.toString (), valueTree }
    {
        jassert (valueTree.getType () == type);
    }

    MAKE_VALUE_MEMBER (uint32_t, data0, 0);
    MAKE_VALUE_MEMBER (uint32_t, data1, 0);
    MAKE_VALUE_MEMBER (uint32_t, data2, 0);
    MAKE_VALUE_MEMBER (uint32_t, data3, 0);
    MAKE_VALUE_MEMBER (double, timestamp, 0.0);
    MAKE_VALUE_MEMBER (int, endpointIndex, 0);

    MAKE_BITFIELD (int, messageType, 0, 4, 28);
};
