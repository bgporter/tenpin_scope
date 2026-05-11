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
#include <utility>

#include "handler/umpHandler.h"
#include "model/eventList.h"
#include "model/sysex/sysex8Message.h"
#include "model/ump/sysex8.h"

class Sysex8Builder : public UmpHandler
{
public:
    using DeferFn = std::function<void (Event&)>;

    Sysex8Builder (EventList theEventList, DeferFn deferFn);

private:
    Handler::Result onSysex8CompleteEvent (const UmpEvent& e) override;
    Handler::Result onSysex8StartEvent    (const UmpEvent& e) override;
    Handler::Result onSysex8ContinueEvent (const UmpEvent& e) override;
    Handler::Result onSysex8EndEvent      (const UmpEvent& e) override;

    void createBuffer    (const Sysex8Event& e);
    bool appendData      (const Sysex8Event& e);
    void completeMessage (const Sysex8Event& e);

    EventList                                   eventList;
    DeferFn                                     deferFn;
    std::map<std::pair<int, int>, Buffer::Ptr>  inProgressBuffers;
};
