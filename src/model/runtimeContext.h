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

#include <cello/cello.h>

#include "eventList.h"
#include "midiProperties.h"

class RuntimeContext : public cello::Object
{
public:
    static const inline juce::Identifier type { "RuntimeContext" };

    /**
     * @brief Construct a new RuntimeContext object
     *
     * @param parentOrSelf If the type of the object we're passed matches our type,
     * we'll wrap that directly; otherwise we will look for (or create) a defaulted
     * instance of our type as a child of the passed object.
     */
    RuntimeContext (const cello::Object& parentOrSelf)
    : cello::Object { type.toString (), parentOrSelf }
    {
        // properties in the runtime context are not undoable.
        setUndoManager (nullptr);
        // mirror the same constraints as PersistentContext so drag values stay valid
        sidebarWidth.onSet = [] (int v) { return std::clamp (v, 150, 400); };
        col1Width.onSet    = [] (int v) { return std::clamp (v, 20, 200); };
        col2Width.onSet    = [] (int v) { return std::clamp (v, 20, 200); };
    }

    /// a 'temp' var where we can store the *last* sidebar width value during dragging.
    MAKE_VALUE_MEMBER (int, sidebarWidth, 200);
    /// @brief Cached event list column widths for use during drag operations.
    MAKE_VALUE_MEMBER (int, col1Width, 90);
    MAKE_VALUE_MEMBER (int, col2Width, 140);
    EventList eventList;
    MidiProperties midiProperties { *this };
};
