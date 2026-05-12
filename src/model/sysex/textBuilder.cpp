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

#include "textBuilder.h"

namespace
{
// Append up to maxBytes bytes from a text event packet, stopping at the first null byte.
template <typename TextEvent>
void appendPacketBytes (std::vector<uint8_t>& out, const TextEvent& e)
{
    for (int i = 0; i < TextEvent::maxBytes; ++i)
    {
        const uint8_t b = e[i];
        if (b == 0)
            break;
        out.push_back (b);
    }
}
} // namespace

TextBuilder::TextBuilder (EventList theEventList, DeferFn deferFn_)
: eventList { theEventList }
, deferFn { std::move (deferFn_) }
{
    eventList.onChildAdded = [this] (juce::ValueTree& vt, int, int)
    {
        if (vt.hasType (UmpEvent::type))
        {
            UmpEvent event { vt };
            UmpHandler::handle (event);
        }
    };
}

Handler::Result TextBuilder::onMetadataTextEvent (const UmpEvent& e)
{
    return handleFlexTextPacket (FlexDataTextEvent { e });
}

Handler::Result TextBuilder::onPerformanceTextEvent (const UmpEvent& e)
{
    return handleFlexTextPacket (FlexDataTextEvent { e });
}

Handler::Result TextBuilder::onEndpointNameNotificationEvent (const UmpEvent& e)
{
    return handleStreamTextPacket (StreamTextEvent { e });
}

Handler::Result TextBuilder::onProductInstanceIdEvent (const UmpEvent& e)
{
    return handleStreamTextPacket (StreamTextEvent { e });
}

Handler::Result TextBuilder::onFunctionBlockNameNotificationEvent (const UmpEvent& e)
{
    return handleStreamTextPacket (StreamTextEvent { e });
}

Handler::Result TextBuilder::handleFlexTextPacket (const FlexDataTextEvent& e)
{
    const auto typeName = TextMessage::typeForFlexStatus (static_cast<int> (e.statusBank.get ()),
                                                          e.status.get ());
    if (typeName.isEmpty ())
        return Handler::Result::notHandled;

    const FlexKey key { e.group.get (), e.channel.get (),
                        static_cast<int> (e.statusBank.get ()), e.status.get () };
    const auto fmt = e.format.get ();

    if (fmt == FlexDataFormat::complete || fmt == FlexDataFormat::start)
    {
        flexInProgress.erase (key);
        FlexInProgress ip;
        ip.group      = e.group.get ();
        ip.channel    = e.channel.get ();
        ip.address    = static_cast<int> (e.address.get ());
        ip.statusBank = static_cast<int> (e.statusBank.get ());
        ip.status     = e.status.get ();
        appendPacketBytes (ip.bytes, e);

        if (fmt == FlexDataFormat::complete)
        {
            completeFlexMessage (e, key, ip);
        }
        else
        {
            flexInProgress[key] = std::move (ip);
        }
    }
    else // continue_ or end
    {
        auto it = flexInProgress.find (key);
        if (it == flexInProgress.end ())
            return Handler::Result::notHandled;

        appendPacketBytes (it->second.bytes, e);

        if (fmt == FlexDataFormat::end)
        {
            completeFlexMessage (e, key, it->second);
            flexInProgress.erase (it);
        }
    }
    return Handler::Result::ok;
}

Handler::Result TextBuilder::handleStreamTextPacket (const StreamTextEvent& e)
{
    const auto typeName = TextMessage::typeForStreamStatus (e.status.get ());
    if (typeName.isEmpty ())
        return Handler::Result::notHandled;

    const StreamKey key { e.status.get (), e.functionBlockNumber.get () };
    const auto fmt = e.format.get ();

    if (fmt == StreamFormat::complete || fmt == StreamFormat::start)
    {
        streamInProgress.erase (key);
        StreamInProgress ip;
        ip.status              = e.status.get ();
        ip.functionBlockNumber = e.functionBlockNumber.get ();
        appendPacketBytes (ip.bytes, e);

        if (fmt == StreamFormat::complete)
        {
            completeStreamMessage (e, key, ip);
        }
        else
        {
            streamInProgress[key] = std::move (ip);
        }
    }
    else // continue_ or end
    {
        auto it = streamInProgress.find (key);
        if (it == streamInProgress.end ())
            return Handler::Result::notHandled;

        appendPacketBytes (it->second.bytes, e);

        if (fmt == StreamFormat::end)
        {
            completeStreamMessage (e, key, it->second);
            streamInProgress.erase (it);
        }
    }
    return Handler::Result::ok;
}

void TextBuilder::completeFlexMessage (const FlexDataTextEvent& e,
                                        const std::tuple<int,int,int,int>& key,
                                        FlexInProgress& ip)
{
    const auto typeName = TextMessage::typeForFlexStatus (std::get<2> (key), std::get<3> (key));
    const juce::String text = juce::String::fromUTF8 (
        reinterpret_cast<const char*> (ip.bytes.data ()),
        static_cast<int> (ip.bytes.size ()));

    TextMessage msg { typeName, ip.group, ip.channel, ip.address, ip.statusBank, ip.status, text };
    msg.timestamp     = e.timestamp.get ();
    msg.endpointIndex = e.endpointIndex.get ();
    msg.endpointName  = e.endpointName.get ();
    msg.isReceived    = e.isReceived.get ();

    deferFn (msg);
}

void TextBuilder::completeStreamMessage (const StreamTextEvent& e,
                                          const std::pair<int,int>& key,
                                          StreamInProgress& ip)
{
    const auto typeName = TextMessage::typeForStreamStatus (key.first);
    const juce::String text = juce::String::fromUTF8 (
        reinterpret_cast<const char*> (ip.bytes.data ()),
        static_cast<int> (ip.bytes.size ()));

    TextMessage msg { typeName, ip.functionBlockNumber, text };
    msg.status        = ip.status;
    msg.timestamp     = e.timestamp.get ();
    msg.endpointIndex = e.endpointIndex.get ();
    msg.endpointName  = e.endpointName.get ();
    msg.isReceived    = e.isReceived.get ();

    deferFn (msg);
}

#if RUN_UNIT_TESTS
#include "test/test_TextBuilder.inl"
#endif
