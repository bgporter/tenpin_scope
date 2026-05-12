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
#include "model/sysex/mdsMessage.h"
#include "model/ump/sysex8.h"

class MdsBuilder : public UmpHandler
{
public:
    using DeferFn = std::function<void (Event&)>;

    MdsBuilder (EventList theEventList, DeferFn deferFn);

private:
    Handler::Result onMixedDataSetHeaderEvent  (const UmpEvent& e) override;
    Handler::Result onMixedDataSetPayloadEvent (const UmpEvent& e) override;

    struct InProgress
    {
        Buffer::Ptr buffer;
        int numChunks      { 0 };
        int numValidBytes  { 0 };
        int chunksReceived { 0 };
        int manufacturerId { 0 };
        int deviceId       { 0 };
        int subId1         { 0 };
        int subId2         { 0 };
    };

    void completeMessage (const MixedDataSetPayloadEvent& e, const std::pair<int, int>& key);

    EventList                                    eventList;
    DeferFn                                      deferFn;
    std::map<std::pair<int, int>, InProgress>    inProgressBuffers;
};
