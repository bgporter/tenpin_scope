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
#include <map>

#include "handler/umpHandler.h"
#include "model/eventList.h"
#include "model/sysex/sysex7Message.h"
#include "model/ump/sysex7.h"

class Sysex7Builder : public UmpHandler
{
public:
    Sysex7Builder (EventList theEventList);

private:
    Handler::Result onSysex7CompleteEvent (const UmpEvent& e) override;
    Handler::Result onSysex7StartEvent    (const UmpEvent& e) override;
    Handler::Result onSysex7ContinueEvent (const UmpEvent& e) override;
    Handler::Result onSysex7EndEvent      (const UmpEvent& e) override;

    // Step 3: start a new buffer for e's group, discarding any incomplete prior one
    void createBuffer   (const Sysex7Event& e);
    // Step 4: append e's data bytes to the in-progress buffer; returns false if none found
    bool appendData     (const Sysex7Event& e);
    // Step 5: assemble a Sysex7Message and add it to the event list
    void completeMessage (const Sysex7Event& e);

    EventList                  eventList;
    std::map<int, Buffer::Ptr> inProgressBuffers;
};
