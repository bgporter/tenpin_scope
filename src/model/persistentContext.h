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

#include "utility/logger.h"
#include "view/eventNameUtils.h"

struct EventViewContext : public cello::Object
{
    static const inline juce::Identifier type { "EventViewContext" };

    EventViewContext (const cello::Object& parentOrSelf)
    : cello::Object { type.toString (), parentOrSelf }
    {
        col1Width.onSet = [] (int v) { return std::clamp (v, 20, 200); };
        col2Width.onSet = [] (int v) { return std::clamp (v, 20, 200); };
        col3Width.onSet = [] (int v) { return std::clamp (v, 20, 400); };
    }

    MAKE_VALUE_MEMBER (float, textHeight, 14.f);

    /// @brief Width of the Time column in the event list view.
    MAKE_VALUE_MEMBER (int, col1Width, 90);
    /// @brief Width of the Endpoint column in the event list view.
    MAKE_VALUE_MEMBER (int, col2Width, 140);
    /// @brief Width of the Event column in the event list view.
    MAKE_VALUE_MEMBER (int, col3Width, 160);

    // Individual cell formatting things.
    MAKE_VALUE_MEMBER (OctaveType, octaveType, OctaveType::Yamaha);
    MAKE_VALUE_MEMBER (ValueFormatType, valueFormatType, ValueFormatType::Hex);
    MAKE_VALUE_MEMBER (int, precision, 2);

    // if neither is set to true, we'll just show hex (raw) data.
    MAKE_VALUE_MEMBER (bool, umpShowParsedData, true);
    MAKE_VALUE_MEMBER (bool, umpShowRawData, true);

    /// @brief Maximum number of data bytes to display for assembled messages (e.g. Sysex7Message).
    MAKE_VALUE_MEMBER (int, maxDataBytes, 32);

    // filter settings for the event list view.
    MAKE_VALUE_MEMBER (bool, showChannelVoice, true);
    MAKE_VALUE_MEMBER (bool, showNoteOnOff, true);
    MAKE_VALUE_MEMBER (bool, showProgramChange, true);
    MAKE_VALUE_MEMBER (bool, showControlChange, true);
    MAKE_VALUE_MEMBER (bool, showPitchBend, true);
    MAKE_VALUE_MEMBER (bool, showPerNoteEvents, true);
    MAKE_VALUE_MEMBER (bool, showChannelPressure, true);

    // top-level message type filters
    MAKE_VALUE_MEMBER (bool, showUtility, true);
    MAKE_VALUE_MEMBER (bool, showSystemCommonRealtime, true);
    MAKE_VALUE_MEMBER (bool, showSysex7, true);
    MAKE_VALUE_MEMBER (bool, showSysex8, true);
    MAKE_VALUE_MEMBER (bool, showSysex8Packets, true);
    MAKE_VALUE_MEMBER (bool, showMixedData, true);
    MAKE_VALUE_MEMBER (bool, showFlexData, true);
    MAKE_VALUE_MEMBER (bool, showStreamData, true);
    MAKE_VALUE_MEMBER (bool, showUndefined, true);

    // filter settings for data granularity
    // show individual UMP packets? IF nothing else is enabled, we'll just show the raw UMP data.
    MAKE_VALUE_MEMBER (bool, showUmpData, true);
    // display combined sysex / mixed data as a unit when complete.
    MAKE_VALUE_MEMBER (bool, showCombinedData, true);
    // show MIDI CI messages as parsed data (when complete)
    MAKE_VALUE_MEMBER (bool, showMidiCI, true);
    // show complete property exchange messages as parsed data (when complete)
    MAKE_VALUE_MEMBER (bool, showPropertyExchange, true);
};

/**
 * @class PersistentContext
 * @brief This class is used to store the persistent context of the application.
 * It is a child of the AppContext class and is used to store the persistent
 * context of the application in a .prefs file.
 *
 * The main app code will create an instance of this and add it as a child
 * of the AppContext object.
 *
 * We will load that file into this object at app startup time, and periodically
 * check to see if it needs to be saved to disk, and save it if needed.
 *
 */
class PersistentContext : public cello::Object
{
public:
    static const inline juce::Identifier type { "_10PinScope" };
    /**
     * @brief Construct a new PersistentContext object
     *
     * @param parentOrSelf If the type of the object we're passed matches our type,
     * we'll wrap that directly; otherwise we will look for (or create) a defaulted
     * instance of our type as a child of the passed object.
     */
    PersistentContext (const cello::Object& parentOrSelf)
    : cello::Object { type.toString (), parentOrSelf }
    {
        // restrict sidebar width to a reasonable range
        sidebarWidth.onSet = [] (int newValue) { return std::clamp (newValue, 150, 400); };
    }

    PersistentContext (const juce::File& file)
    : cello::Object { type.toString (), file }
    {
    }

    juce::Result save (const juce::File& file)
    {
        juce::XmlElement::TextFormat textFormat;
        textFormat.lineWrapLength = 0; // don't wrap the XML, to make it easier to read in the prefs file.

        auto ok = cello::Object::save (file, PersistentContext::FileFormat::xml, textFormat);
        if (ok)
        {
            // we use the undo manager to track changes to the prefs file -- if there's nothing
            // undoable, then we know we haven't been changed.
            clearUndoHistory ();
            return ok;
        }
        return juce::Result::fail ("Failed to save file " + file.getFullPathName () + ": " + ok.getErrorMessage ());
    }

    /**
     * @brief Save the prefs file if there are any changes. We use the undo manager to track
     * changes to the prefs file -- if there's nothing undoable, then we know we haven't been changed.
     *
     * @param file
     * @return juce::Result
     */
    juce::Result saveIfNeeded (const juce::File& file) { return canUndo () ? save (file) : juce::Result::ok (); }

    MAKE_VALUE_MEMBER (juce::String, windowState, {});
    MAKE_VALUE_MEMBER (int, sidebarWidth, 200);
    MAKE_VALUE_MEMBER (LogWriter::Level, logLevel, LogWriter::Level::info);
    /// @brief True when we're in the middle of a drag operation, to prevent saving prefs mid-drag.
    MAKE_VALUE_MEMBER (bool, dragging, false);

    EventViewContext eventViewContext { *this };
};
