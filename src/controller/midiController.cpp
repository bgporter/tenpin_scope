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

#include "midiController.h"

#include "utility/logger.h"

MidiController::MidiController (juce::StringRef sessionName, const AppContext& appContext)
: runtimeContext { appContext }
, midiProperties { runtimeContext }
, endpoints { juce::ump::Endpoints::getInstance () }
, session { endpoints->makeSession (sessionName) }
{
    if (!midiProperties.wasInitialized ())
    {
        ERROR_ ("MidiProperties not initialized");
    }

    if (!endpoints)
    {
        ERROR_ ("Endpoints not initialized");
        return;
        // !!! We should throw an exception here
    }

    if (!session.isAlive ())
    {
        ERROR_ ("Failed to create session");
        return;
        // !!! We should throw an exception here
    }

    const auto endpointIds = endpoints->getEndpoints ();
    int i                  = 0;
    for (const auto& endpointId : endpointIds)
    {
        if (auto endpoint = endpoints->getEndpoint (endpointId))
        {
            DBG ("Found device: " << endpoint->getName ());
            INFO_ ({
                {  "msg",                          "Device found"},
                { "name",                    endpoint->getName ()},
                {"srcId", endpointId.get (juce::ump::IOKind::src)},
                {"dstId", endpointId.get (juce::ump::IOKind::dst)}
            });
            ++i;
            // midiProperties.addEndpoint (endpoint);
            // create a new EndpointController object (that we'll own), which will
            // itself create a new MidiEndpointProperties object added into the properties
            // tree for the rest of the application to use.
        }
    }
    INFO_ ({
        {  "msg", "Found MIDI endpoints"},
        {"count",                      i}
    });
    endpoints->addListener (*this);
}

MidiController::~MidiController ()
{
    endpoints->removeListener (*this);
}
