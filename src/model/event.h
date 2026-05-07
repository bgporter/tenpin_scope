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

struct Event : public cello::Object
{
    Event (const juce::String& typeName, const cello::Object* parent = nullptr)
    : cello::Object { typeName, parent }
    {
    }

    Event (const juce::String& typeName, juce::ValueTree valueTree)
    : cello::Object { typeName, valueTree }
    {
    }

    MAKE_VALUE_MEMBER (double, timestamp, 0.0);
    MAKE_VALUE_MEMBER (int, endpointIndex, 0);
    MAKE_VALUE_MEMBER (juce::String, endpointName, "");
    MAKE_VALUE_MEMBER (bool, isReceived, false);
};
