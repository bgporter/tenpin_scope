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

#include "eventListViewMsgHandler.h"

#include "view/eventView.h"

EventListViewMsgHandler::EventListViewMsgHandler () {}

EventListViewMsgHandler::~EventListViewMsgHandler () {}

Handler::Result EventListViewMsgHandler::handle (const Event& e, void* ctx)
{
    if (e.getTypeName () == Sysex7Message::type.toString ())
    {
        Sysex7Message msg (e);
        auto* dispCtx   = static_cast<DispatchContext*> (ctx);
        auto* eventView = dispCtx->view;
        const int width = dispCtx->width;

        eventView->setColors (juce::Colours::darkgrey, juce::Colours::white, juce::Colours::white, juce::Colours::grey);
        eventView->setTime (juce::String (static_cast<double> (msg.timestamp), 3));
        const auto endpointStr = juce::String (msg.endpointName) + " " + (msg.isReceived ? "Rx" : "Tx");
        eventView->setEndpoint (endpointStr);
        eventView->setEvent ("Sysex7 Message");
        if (auto buf = msg.data.get ())
            eventView->addValue ("bytes", juce::String (buf->size ()));
        eventView->sizeToWidth (width);
        return Handler::Result::ok;
    }
    return Handler::Result::notHandled;
}

Handler::Result EventListViewMsgHandler::handle (const Event& e)
{
    return handle (e, nullptr);
}
