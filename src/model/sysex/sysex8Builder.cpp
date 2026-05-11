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

#include "sysex8Builder.h"

Sysex8Builder::Sysex8Builder (EventList theEventList, DeferFn deferFn_)
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

Handler::Result Sysex8Builder::onSysex8StartEvent (const UmpEvent& e)
{
    Sysex8Event sysex { e };
    createBuffer (sysex);
    appendData (sysex);
    return Handler::Result::ok;
}

Handler::Result Sysex8Builder::onSysex8ContinueEvent (const UmpEvent& e)
{
    Sysex8Event sysex { e };
    if (!appendData (sysex))
        return Handler::Result::notHandled;
    return Handler::Result::ok;
}

Handler::Result Sysex8Builder::onSysex8EndEvent (const UmpEvent& e)
{
    Sysex8Event sysex { e };
    if (!appendData (sysex))
        return Handler::Result::notHandled;
    completeMessage (sysex);
    return Handler::Result::ok;
}

Handler::Result Sysex8Builder::onSysex8CompleteEvent (const UmpEvent& e)
{
    Sysex8Event sysex { e };
    createBuffer (sysex);
    appendData (sysex);
    completeMessage (sysex);
    return Handler::Result::ok;
}

void Sysex8Builder::createBuffer (const Sysex8Event& e)
{
    const auto key = std::make_pair (e.group.get (), e.streamId.get ());
    if (inProgressBuffers.contains (key))
        inProgressBuffers.erase (key);
    inProgressBuffers[key] = new Buffer ();
}

bool Sysex8Builder::appendData (const Sysex8Event& e)
{
    const auto key = std::make_pair (e.group.get (), e.streamId.get ());
    auto it        = inProgressBuffers.find (key);
    if (it == inProgressBuffers.end ())
        return false;

    // numBytes includes the streamId byte in its count, so actual data = numBytes - 1
    const int n = e.numBytes.get () - 1;
    for (int i = 0; i < n; ++i)
        it->second->append (static_cast<uint8_t> (e[i]));
    return true;
}

void Sysex8Builder::completeMessage (const Sysex8Event& e)
{
    const auto key = std::make_pair (e.group.get (), e.streamId.get ());
    auto it        = inProgressBuffers.find (key);
    if (it == inProgressBuffers.end ())
        return;

    Sysex8Message msg { MidiNibble { e.group.get () }, e.streamId.get (), it->second };
    msg.timestamp     = e.timestamp.get ();
    msg.endpointIndex = e.endpointIndex.get ();
    msg.endpointName  = e.endpointName.get ();
    msg.isReceived    = e.isReceived.get ();
    inProgressBuffers.erase (it);

    deferFn (msg);
}

#if RUN_UNIT_TESTS
#include "test/test_Sysex8Builder.inl"
#endif
