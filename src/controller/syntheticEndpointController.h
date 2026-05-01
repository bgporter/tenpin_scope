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

#include "model/midiEndpointProperties.h"
#include "model/midiProperties.h"
#include "model/ump/umpEvent.h"

struct TestEvent
{
    uint32_t deltaTimeMs;
    UmpEvent event;
};

class TestEventList
{
public:
    void addEvent (uint32_t deltaTimeMs, UmpEvent event)
    {
        eventList.push_back ({ deltaTimeMs, std::move (event) });
    }

    std::vector<TestEvent> eventList;
};

class SyntheticEndpointController
{
public:
    SyntheticEndpointController (const MidiProperties& midiProperties, const juce::String& name);
    ~SyntheticEndpointController () = default;

    /** Called by MidiController::timerCallback() at ~30 Hz on the message thread. */
    void processUmpEvents ();

private:
    void buildDefaultEventList ();
    void startPlayback ();

    MidiProperties         midiProperties;
    MidiEndpointProperties midiEndpointProperties;
    TestEventList          eventList;

    size_t      nextEventIndex { 0 };
    juce::int64 nextFireTimeMs { 0 };
    bool        playing        { false };
    static inline double startTime { -1.0 };
};
