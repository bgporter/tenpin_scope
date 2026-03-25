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

#include "ump/umpEvent.h"
#include "utility/variantConverters.h"

struct EventList : public cello::Object
{
public:
    static const inline juce::Identifier type { "EventList" };

    /**
     * @brief Construct a new EventList object ex nihilo
     */
    EventList (juce::StringRef typeName = type.toString ())
    : cello::Object { typeName, nullptr }
    {
    }

    /**
     * @brief Construct a new EventList object from a value tree (as we
     * would after e.g. performing a query.)
     */
    EventList (juce::ValueTree valueTree)
    : cello::Object { type.toString (), valueTree }
    {
    }

    /**
     * @brief Construct a new EventList as a named child of another object
     * (e.g. "received" or "transmitted" for MidiEndpointProperties)
     */
    EventList (juce::StringRef typeName, const cello::Object& parentOrSelf)
    : cello::Object { typeName, parentOrSelf }
    {
    }

    EventList (const EventList& rhs)
    : cello::Object { rhs }
    {
    }

    void addEvent (UmpEvent& event)
    {
        append (&event);
        count++;
    }

    void clear ()
    {
        data.removeAllChildren (getUndoManager ());
        count = 0;
    }

    MAKE_VALUE_MEMBER (int, count, 0);
    MAKE_VALUE_MEMBER (bool, isRebuilding, false);
};
