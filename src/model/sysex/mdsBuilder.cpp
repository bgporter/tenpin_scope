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

#include "mdsBuilder.h"

MdsBuilder::MdsBuilder (EventList theEventList, DeferFn deferFn_)
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

Handler::Result MdsBuilder::onMixedDataSetHeaderEvent (const UmpEvent& e)
{
    MixedDataSetHeaderEvent hdr { e };
    const auto key = std::make_pair (hdr.group.get (), hdr.mdsId.get ());

    inProgressBuffers.erase (key);

    InProgress ip;
    ip.buffer         = new Buffer ();
    ip.numChunks      = hdr.numChunks.get ();
    ip.numValidBytes  = hdr.numValidBytes.get ();
    ip.manufacturerId = hdr.manufacturerId.get ();
    ip.deviceId       = hdr.deviceId.get ();
    ip.subId1         = hdr.subId1.get ();
    ip.subId2         = hdr.subId2.get ();
    inProgressBuffers[key] = std::move (ip);

    return Handler::Result::ok;
}

Handler::Result MdsBuilder::onMixedDataSetPayloadEvent (const UmpEvent& e)
{
    MixedDataSetPayloadEvent pld { e };
    const auto key = std::make_pair (pld.group.get (), pld.mdsId.get ());

    auto it = inProgressBuffers.find (key);
    if (it == inProgressBuffers.end ())
        return Handler::Result::notHandled;

    ++it->second.chunksReceived;
    const bool isLast = (it->second.chunksReceived == it->second.numChunks);

    // Final chunk only carries the remaining valid bytes; all prior chunks carry 14.
    const int bytesToAppend = isLast
        ? (it->second.numValidBytes - (it->second.numChunks - 1) * 14)
        : 14;
    for (int i = 0; i < bytesToAppend; ++i)
        it->second.buffer->append (static_cast<uint8_t> (pld[i]));

    if (isLast)
        completeMessage (pld, key);

    return Handler::Result::ok;
}

void MdsBuilder::completeMessage (const MixedDataSetPayloadEvent& pld, const std::pair<int, int>& key)
{
    auto it = inProgressBuffers.find (key);
    if (it == inProgressBuffers.end ())
        return;

    const auto& ip = it->second;
    MdsMessage msg { MidiNibble { key.first }, key.second,
                     ip.manufacturerId, ip.deviceId, ip.subId1, ip.subId2,
                     ip.numChunks, ip.buffer };
    msg.timestamp     = pld.timestamp.get ();
    msg.endpointIndex = pld.endpointIndex.get ();
    msg.endpointName  = pld.endpointName.get ();
    msg.isReceived    = pld.isReceived.get ();
    inProgressBuffers.erase (it);

    deferFn (msg);
}

#if RUN_UNIT_TESTS
#include "test/test_MdsBuilder.inl"
#endif
