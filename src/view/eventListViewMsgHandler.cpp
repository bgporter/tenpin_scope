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

#include "model/persistentContext.h"
#include "palette.h"
#include "view/eventNameUtils.h"
#include "view/eventView.h"

EventListViewMsgHandler::EventListViewMsgHandler (AppContext& theAppContext)
: appContext { theAppContext }
{
}

EventListViewMsgHandler::~EventListViewMsgHandler () {}

Handler::Result EventListViewMsgHandler::handle (const Event& e, void* ctx)
{
    if (e.getTypeName () == Sysex7Message::type.toString ())
    {
        Sysex7Message msg (e);
        auto* dispCtx   = static_cast<DispatchContext*> (ctx);
        auto* eventView = dispCtx->view;
        const int width = dispCtx->width;

        Palette pal { PersistentContext { appContext } };
        PersistentContext pc { appContext };
        const auto rawFormat = pc.eventViewContext.valueFormatType.get ();
        const auto formatType =
            (rawFormat == ValueFormatType::Integer) ? ValueFormatType::Integer : ValueFormatType::Hex;

        eventView->setColors (static_cast<juce::Colour> (pal.umpBackground.get ()).brighter (0.1f),
                              pal.sysex7Label.get (), pal.sysex7Value.get (), pal.outline.get ());
        eventView->setTime (juce::String (static_cast<double> (msg.timestamp), 3));
        const auto endpointStr = juce::String (msg.endpointName) + " " + (msg.isReceived ? "Rx" : "Tx");
        eventView->setEndpoint (endpointStr);
        eventView->setEvent ("Sysex7 Message");

        eventView->addValue ("grp", juce::String (static_cast<int> (msg.group)));

        if (auto buf = msg.data.get ())
        {
            eventView->addValue ("data", "");
            const size_t maxBytes     = static_cast<size_t> (pc.eventViewContext.maxDataBytes.get ());
            const size_t displayCount = std::min (buf->size (), maxBytes);
            for (size_t i = 0; i < displayCount; ++i)
            {
                auto val { formatValue ((*buf)[i], 8, formatType, 2, 0.f, 1.f, i > 0) };
                if (i > 0)
                    val = " " + val;
                eventView->addValue ("", val);
            }
            if (buf->size () > maxBytes)
            {
                const auto remaining = static_cast<int> (buf->size () - maxBytes);
                eventView->addValue ("", juce::String::formatted ("(+%d bytes...)", remaining));
            }
        }

        eventView->sizeToWidth (width);
        return Handler::Result::ok;
    }

    if (e.getTypeName () == Sysex8Message::type.toString ())
    {
        Sysex8Message msg (e);
        auto* dispCtx   = static_cast<DispatchContext*> (ctx);
        auto* eventView = dispCtx->view;
        const int width = dispCtx->width;

        Palette pal { PersistentContext { appContext } };
        PersistentContext pc { appContext };
        const auto rawFormat = pc.eventViewContext.valueFormatType.get ();
        const auto formatType =
            (rawFormat == ValueFormatType::Integer) ? ValueFormatType::Integer : ValueFormatType::Hex;

        eventView->setColors (static_cast<juce::Colour> (pal.umpBackground.get ()).brighter (0.1f),
                              pal.sysex8Label.get (), pal.sysex8Value.get (), pal.outline.get ());
        eventView->setTime (juce::String (static_cast<double> (msg.timestamp), 3));
        const auto endpointStr = juce::String (msg.endpointName) + " " + (msg.isReceived ? "Rx" : "Tx");
        eventView->setEndpoint (endpointStr);
        eventView->setEvent ("Sysex8 Message");

        eventView->addValue ("grp", juce::String (static_cast<int> (msg.group)));
        eventView->addValue ("sid", juce::String (static_cast<int> (msg.streamId)));

        if (auto buf = msg.data.get ())
        {
            eventView->addValue ("data", "");
            const size_t maxBytes     = static_cast<size_t> (pc.eventViewContext.maxDataBytes.get ());
            const size_t displayCount = std::min (buf->size (), maxBytes);
            for (size_t i = 0; i < displayCount; ++i)
            {
                auto val { formatValue ((*buf)[i], 8, formatType, 2, 0.f, 1.f, i > 0) };
                if (i > 0)
                    val = " " + val;
                eventView->addValue ("", val);
            }
            if (buf->size () > maxBytes)
            {
                const auto remaining = static_cast<int> (buf->size () - maxBytes);
                eventView->addValue ("", juce::String::formatted ("(+%d bytes...)", remaining));
            }
        }

        eventView->sizeToWidth (width);
        return Handler::Result::ok;
    }

    return Handler::Result::notHandled;
}

Handler::Result EventListViewMsgHandler::handle (const Event& e)
{
    return handle (e, nullptr);
}
