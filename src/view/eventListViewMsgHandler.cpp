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

#include "model/sysex/mdsMessage.h"
#include "model/sysex/sysex7Message.h"
#include "model/sysex/sysex8Message.h"
#include "model/sysex/textMessage.h"
#include "palette.h"
#include "view/dispatchContext.h"
#include "view/eventNameUtils.h"
#include "view/eventView.h"

EventListViewMsgHandler::EventListViewMsgHandler (AppContext& theAppContext)
: appContext { theAppContext }
, pc { theAppContext }
{
}

EventListViewMsgHandler::~EventListViewMsgHandler () {}

Handler::Result EventListViewMsgHandler::handle (const Event& e, void* ctx)
{
    if (ctx)
    {
        auto* dispCtx = static_cast<DispatchContext*> (ctx);
        eventView    = dispCtx->view;
        currentWidth = dispCtx->width;
    }

    if (eventView)
    {
        Palette pal { pc };
        const auto type = e.getType ();
        juce::Colour labelColor, valueColor;

        if (type == Sysex7Message::type)
        {
            labelColor = pal.sysex7Label.get ();
            valueColor = pal.sysex7Value.get ();
        }
        else if (type == Sysex8Message::type || type == MdsMessage::type)
        {
            labelColor = pal.sysex8Label.get ();
            valueColor = pal.sysex8Value.get ();
        }
        else if (TextMessage::isFlexDataTextMessage (type))
        {
            labelColor = pal.flexDataLabel.get ();
            valueColor = pal.flexDataValue.get ();
        }
        else
        {
            labelColor = pal.streamLabel.get ();
            valueColor = pal.streamValue.get ();
        }

        eventView->setColors (static_cast<juce::Colour> (pal.umpBackground.get ()).brighter (0.1f),
                              labelColor, valueColor, pal.outline.get ());
    }

    return MessageHandler::handle (e, ctx);
}

Handler::Result EventListViewMsgHandler::preDispatch (const Event& e)
{
    if (!eventView)
        return Handler::Result::notHandled;
    eventView->setTime (formatTime (e.timestamp));
    const auto endpointStr = juce::String (e.isReceived ? "src: " : "dst: ") + juce::String (e.endpointName);
    eventView->setEndpoint (endpointStr);
    return Handler::Result::ok;
}

Handler::Result EventListViewMsgHandler::postDispatch (const Event& /*e*/, Handler::Result pendingResult)
{
    if (pendingResult == Handler::Result::ok)
        eventView->sizeToWidth (currentWidth);
    return pendingResult;
}

Handler::Result EventListViewMsgHandler::onSysex7Message (const Event& e)
{
    Sysex7Message msg (e);
    const auto rawFormat  = pc.eventViewContext.valueFormatType.get ();
    const auto formatType = (rawFormat == ValueFormatType::Integer) ? ValueFormatType::Integer : ValueFormatType::Hex;

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
    return Handler::Result::ok;
}

Handler::Result EventListViewMsgHandler::onSysex8Message (const Event& e)
{
    Sysex8Message msg (e);
    const auto rawFormat  = pc.eventViewContext.valueFormatType.get ();
    const auto formatType = (rawFormat == ValueFormatType::Integer) ? ValueFormatType::Integer : ValueFormatType::Hex;

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
    return Handler::Result::ok;
}

Handler::Result EventListViewMsgHandler::onMdsMessage (const Event& e)
{
    MdsMessage msg (e);
    const auto rawFormat  = pc.eventViewContext.valueFormatType.get ();
    const auto formatType = (rawFormat == ValueFormatType::Integer) ? ValueFormatType::Integer : ValueFormatType::Hex;

    eventView->setEvent ("MDS Message");
    eventView->addValue ("grp",  juce::String (static_cast<int> (msg.group)));
    eventView->addValue ("id",   juce::String (static_cast<int> (msg.mdsId)));
    eventView->addValue ("mfr",  juce::String::toHexString (static_cast<int> (msg.manufacturerId)));
    eventView->addValue ("dev",  juce::String::toHexString (static_cast<int> (msg.deviceId)));
    eventView->addValue ("sub1", juce::String::toHexString (static_cast<int> (msg.subId1)));
    eventView->addValue ("sub2", juce::String::toHexString (static_cast<int> (msg.subId2)));

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
    return Handler::Result::ok;
}

Handler::Result EventListViewMsgHandler::onFlexDataTextMessage (const Event& e)
{
    TextMessage msg (e);
    eventView->setEvent (TextMessage::displayName (e.getType ()));
    eventView->addValue ("grp",  juce::String (static_cast<int> (msg.group)));
    eventView->addValue ("ch",   juce::String (static_cast<int> (msg.channel)));
    eventView->addValue ("text", static_cast<juce::String> (msg.text));
    return Handler::Result::ok;
}

Handler::Result EventListViewMsgHandler::onStreamTextMessage (const Event& e)
{
    TextMessage msg (e);
    eventView->setEvent (TextMessage::displayName (e.getType ()));
    if (e.getType () == TextMessage::typeFunctionBlockName)
        eventView->addValue ("fb", juce::String (static_cast<int> (msg.functionBlockNumber)));
    eventView->addValue ("text", static_cast<juce::String> (msg.text));
    return Handler::Result::ok;
}
