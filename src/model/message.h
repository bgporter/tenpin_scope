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

#include "model/event.h"

/**
 * @brief Intermediate base class for assembled multi-packet messages.
 *
 * Provides group and channel as stored cello::Value<int> properties (0-based,
 * clamped to 0-15), plus userGroup/userChannel computed values (1-based) for
 * display. Derived classes hide channel or group (via private using-declarations)
 * when the message type doesn't use them.
 */
struct MessageBase : public Event
{
    MessageBase (const juce::String& typeName, const cello::Object* parent = nullptr)
    : Event { typeName, parent }
    {
        init ();
    }

    MessageBase (const juce::String& typeName, juce::ValueTree valueTree)
    : Event { typeName, valueTree }
    {
        init ();
    }

    MAKE_VALUE_MEMBER (int, group,   {});
    MAKE_VALUE_MEMBER (int, channel, {});

    MAKE_COMPUTED_VALUE_MEMBER (
        int, userGroup, [this] () -> int { return group.get () + 1; }, [this] (const int& val) { group = val - 1; });

    MAKE_COMPUTED_VALUE_MEMBER (
        int, userChannel, [this] () -> int { return channel.get () + 1; },
        [this] (const int& val) { channel = val - 1; });

private:
    void init ()
    {
        group.onSet   = [] (const int& v) { return std::clamp (v, 0, 15); };
        channel.onSet = [] (const int& v) { return std::clamp (v, 0, 15); };
    }
};
