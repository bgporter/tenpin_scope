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
#include <tuple>
#include <vector>

#include "handler/umpHandler.h"
#include "model/eventList.h"
#include "model/sysex/textMessage.h"
#include "model/ump/flexData.h"
#include "model/ump/stream.h"

/**
 * @brief Assembles multi-packet FlexData text events and stream text events
 * into individual TextMessage assembled rows.
 *
 * Handles all FlexData metadata text (12 types), performance text (4 types),
 * and stream text (endpoint name, product instance ID, function block name).
 *
 * FlexData streams are keyed by (group, channel, statusBank, status).
 * Stream text streams are keyed by (status, functionBlockNumber).
 */
class TextBuilder : public UmpHandler
{
public:
    using DeferFn = std::function<void (Event&)>;

    TextBuilder (EventList theEventList, DeferFn deferFn);

private:
    Handler::Result onMetadataTextEvent                  (const UmpEvent& e) override;
    Handler::Result onPerformanceTextEvent               (const UmpEvent& e) override;
    Handler::Result onEndpointNameNotificationEvent      (const UmpEvent& e) override;
    Handler::Result onProductInstanceIdEvent             (const UmpEvent& e) override;
    Handler::Result onFunctionBlockNameNotificationEvent (const UmpEvent& e) override;

    Handler::Result handleFlexTextPacket   (const FlexDataTextEvent& e);
    Handler::Result handleStreamTextPacket (const StreamTextEvent& e);

    struct FlexInProgress
    {
        std::vector<uint8_t> bytes;
        int group   { 0 };
        int channel { 0 };
        int address { 0 };
        int statusBank { 0 };
        int status     { 0 };
    };

    struct StreamInProgress
    {
        std::vector<uint8_t> bytes;
        int functionBlockNumber { 0 };
        int status              { 0 };
    };

    void completeFlexMessage   (const FlexDataTextEvent& e,  const std::tuple<int,int,int,int>& key,
                                 FlexInProgress& ip);
    void completeStreamMessage (const StreamTextEvent& e,    const std::pair<int,int>& key,
                                 StreamInProgress& ip);

    EventList eventList;
    DeferFn   deferFn;

    using FlexKey   = std::tuple<int,int,int,int>; // (group, channel, statusBank, status)
    using StreamKey = std::pair<int,int>;           // (status, functionBlockNumber)

    std::map<FlexKey,   FlexInProgress>   flexInProgress;
    std::map<StreamKey, StreamInProgress> streamInProgress;
};
