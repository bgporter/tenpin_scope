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

#include "midiController.h"
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

EndpointController::EndpointController (juce::ump::EndpointId id, MidiController* parent)
: endpointId { id }
, midiEndpointProperties { id }
, parentController { parent }
{
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

void EndpointController::connectEndpoint ()
{
    auto session = parentController->getSession ();
    if (!session->isAlive ())
    {
        ERROR_ ("Session is not alive");
        return;
    }

    auto endpoints              = juce::ump::Endpoints::getInstance ();
    auto endpoint               = endpoints->getEndpoint (endpointId);
    midiEndpointProperties.name = endpoint->getName ();

    // !!! NOTE: borrowed from the JUCE demo code. Doesn't make 100% sense fo r
    // us here; we will be doing CI ourselves, so what we really want to know is
    // whether the endpoint supports comms for that purpose, not the UI hint.
    if (hasFunctionBlockInDirection (*endpoint, ump::IOKind::src))
    {
        input = session->connectInput (endpointId, juce::ump::PacketProtocol::MIDI_2_0);
        if (input.isAlive ())
        {
            input.addConsumer (*this);
            DBG ("Added consumer to input");
        }
    }

    if (hasFunctionBlockInDirection (*endpoint, ump::IOKind::dst))
        output = session->connectOutput (endpointId);

    midiEndpointProperties.isInputAlive  = input.isAlive ();
    midiEndpointProperties.isOutputAlive = output.isAlive ();

    INFO_ ({
        {   "msg",                                           "Connected endpoint"},
        {  "name",                             midiEndpointProperties.name.get ()},
        {    "id",                 midiEndpointProperties.endpointIdString.get ()},
        { "input",  midiEndpointProperties.isInputAlive.get () ? "alive" : "dead"},
        {"output", midiEndpointProperties.isOutputAlive.get () ? "alive" : "dead"}
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
    for (const auto& v : makeRange (b, e))
    {
        midiEndpointProperties.rxCount++;
    }
    TRACE_ ({
        {       "msg",                        "MIDI message received"},
        {      "time",                  juce::String::formatted ("%f",time) },
        {      "name",             midiEndpointProperties.name.get ()                                             },
        {   "rxCount",          midiEndpointProperties.rxCount.get () },
        {   "txCount",          midiEndpointProperties.txCount.get ()                                             },
        {"endpointId", midiEndpointProperties.endpointIdString.get () }
    });
}
