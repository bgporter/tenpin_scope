/*
 MIT License

 Copyright (c) 2026 Brett g Porter

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to so, subject to the following conditions:

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

#include "endpointController.h"

#include "model/ump/umpEvent.h"
#include "utility/logger.h"

namespace
{

bool matches (ump::BlockUiHint ui, ump::IOKind dir)
{
    if (ui == ump::BlockUiHint::bidirectional)
        return true;

    if (dir == ump::IOKind::src)
        return ui == ump::BlockUiHint::sender;

    return ui == ump::BlockUiHint::receiver;
}

bool matches (ump::BlockDirection bd, ump::IOKind dir)
{
    if (bd == ump::BlockDirection::bidirectional)
        return true;

    if (dir == ump::IOKind::src)
        return bd == ump::BlockDirection::sender;
    return bd == ump::BlockDirection::receiver;
}

bool hasFunctionBlockInDirection (const ump::Endpoint& e, ump::IOKind direction)
{
    const auto blocks = e.getBlocks ();

    return std::any_of (blocks.begin (), blocks.end (),
                        [&] (const ump::Block& x)
                        { return matches (x.getUiHint (), direction) || matches (x.getDirection (), direction); });
}
} // namespace

EndpointController::EndpointController (int index, juce::ump::EndpointId id, const MidiProperties& mp)
: endpointIndex { index }
, midiProperties { mp }
, midiEndpointProperties { id }
{
    midiProperties.endpoints.append (&midiEndpointProperties);
}

EndpointController::~EndpointController ()
{
    if (input.isAlive ())
    {
        input.removeConsumer (*this);
    }
    input  = {};
    output = {};
}

void EndpointController::connectEndpoint (juce::ump::Session* session)
{
    if (!session->isAlive ())
    {
        ERROR_ ("Session is not alive");
        return;
    }

    auto endpoints              = juce::ump::Endpoints::getInstance ();
    auto endpointId             = midiEndpointProperties.endpointId.get ();
    auto endpoint               = endpoints->getEndpoint (endpointId);
    midiEndpointProperties.name = endpoint->getName ();

    // !!! NOTE: borrowed from the JUCE demo code. Doesn't make 100% sense for
    // us here; we will be doing CI ourselves, so what we really want to know is
    // whether the endpoint supports comms for that purpose, not the UI hint.
    if (hasFunctionBlockInDirection (*endpoint, ump::IOKind::src))
    {
        input = session->connectInput (endpointId, juce::ump::PacketProtocol::MIDI_2_0);
        if (input.isAlive ())
            input.addConsumer (*this);
    }

    if (hasFunctionBlockInDirection (*endpoint, ump::IOKind::dst))
        output = session->connectOutput (endpointId);

    midiEndpointProperties.isInputAlive  = input.isAlive ();
    midiEndpointProperties.isOutputAlive = output.isAlive ();

    INFO_ ({
        {   "msg",                                    "Connected endpoint"},
        {  "name",                      midiEndpointProperties.name.get ()},
        {    "id",          midiEndpointProperties.endpointIdString.get ()},
        { "input",  midiEndpointProperties.isInputAlive ? "alive" : "dead"},
        {"output", midiEndpointProperties.isOutputAlive ? "alive" : "dead"}
    });
}
void EndpointController::disconnected ()
{
    INFO_ ("Endpoint disconnected");
    input                                = {};
    output                               = {};
    midiEndpointProperties.isInputAlive  = false;
    midiEndpointProperties.isOutputAlive = false;
}

void EndpointController::consume (juce::ump::Iterator b, juce::ump::Iterator e, double time)
{
    int i { 0 };
    for (const auto& v : makeRange (b, e))
    {
        createUmpEvent (v, time);
        ++i;
    }
    DBG ("EndpointController::consume: " << i << " packets queued");
}

void EndpointController::createUmpEvent (const juce::ump::View& packet, double time)
{
    // This runs on MIDI thread - queue raw packet data only
    if (startTime < 0)
        startTime = time;

    const auto elapsed = time - startTime;

    RawPacketData rawData;
    rawData.size          = packet.size ();
    rawData.timestamp     = elapsed;
    rawData.endpointIndex = endpointIndex;

    // Copy the raw uint32 data from the packet
    size_t i = 0;
    for (const auto& dw : packet)
    {
        rawData.data[i++] = dw;
    }

    const juce::ScopedLock lock (queueLock);
    eventQueue.push (rawData);
}

void EndpointController::processUmpEvents ()
{
    jassert (juce::MessageManager::getInstance ()->isThisTheMessageThread ());

    const auto batchStartTime = juce::Time::getMillisecondCounterHiRes ();

    std::queue<RawPacketData> localQueue;
    {
        const juce::ScopedLock lock (queueLock);
        std::swap (localQueue, eventQueue);
    }

    const size_t eventCount = localQueue.size ();
    if (eventCount == 0)
        return;

    while (!localQueue.empty ())
    {
        auto rawData = localQueue.front ();
        localQueue.pop ();

        // Create UmpEvent on message thread from raw data using new constructor
        UmpEvent umpEvent (rawData.data.data (), rawData.size, rawData.timestamp, rawData.endpointIndex);
        umpEvent.endpointName = midiEndpointProperties.name.get ();
        umpEvent.isReceived   = true;
        midiEndpointProperties.received.addEvent (umpEvent);

        //        TRACE_ ({
        //            {        "msg",                       "MIDI message processed"},
        //            {       "time",                  juce::String::formatted ("%f",umpEvent.timestamp.get ()) },
        //            {"messageType",                    umpEvent.messageType.get () }, {       "name",
        //            midiEndpointProperties.name.get () }, {    "rxCount",   midiEndpointProperties.received.count.get
        //            ()                                             }, { "endpointId",
        //            midiEndpointProperties.endpointIdString.get () }
        //        });
    }

    const auto batchEndTime = juce::Time::getMillisecondCounterHiRes ();
    const auto totalTime    = batchEndTime - batchStartTime;
    const auto meanTime     = totalTime / static_cast<double> (eventCount);

    DEBUG_ ({
        {       "msg",                              "Batch processed"},
        {"eventCount",                               (int) eventCount},
        { "totalTime",             juce::String::formatted ("%.3f ms",totalTime) },
        {  "meanTime",             juce::String::formatted ("%.3f ms",                                             meanTime) },
        {      "name",             midiEndpointProperties.name.get ()   },
        {"endpointId", midiEndpointProperties.endpointIdString.get ()        }
    });
}
