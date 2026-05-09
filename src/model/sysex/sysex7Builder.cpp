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

#include "sysex7Builder.h"

Sysex7Builder::Sysex7Builder (EventList theEventList)
: eventList { theEventList }
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

Handler::Result Sysex7Builder::onSysex7StartEvent (const UmpEvent& e)
{
    Sysex7Event sysex { e };
    createBuffer (sysex);
    appendData (sysex);
    return Handler::Result::ok;
}

Handler::Result Sysex7Builder::onSysex7ContinueEvent (const UmpEvent& e)
{
    Sysex7Event sysex { e };
    if (!appendData (sysex))
        return Handler::Result::notHandled;
    return Handler::Result::ok;
}

Handler::Result Sysex7Builder::onSysex7EndEvent (const UmpEvent& e)
{
    Sysex7Event sysex { e };
    if (!appendData (sysex))
        return Handler::Result::notHandled;
    completeMessage (sysex);
    return Handler::Result::ok;
}

Handler::Result Sysex7Builder::onSysex7CompleteEvent (const UmpEvent& e)
{
    Sysex7Event sysex { e };
    createBuffer (sysex);
    appendData (sysex);
    completeMessage (sysex);
    return Handler::Result::ok;
}

void Sysex7Builder::createBuffer (const Sysex7Event& e)
{
    const int groupKey = e.group.get ();
    if (inProgressBuffers.contains (groupKey))
    {
        // TODO: handle incomplete buffer (start without a matching end)
        inProgressBuffers.erase (groupKey);
    }
    inProgressBuffers[groupKey] = new Buffer ();
}

bool Sysex7Builder::appendData (const Sysex7Event& e)
{
    const int groupKey = e.group.get ();
    auto it            = inProgressBuffers.find (groupKey);
    if (it == inProgressBuffers.end ())
        return false;

    const int n = e.numBytes.get ();
    for (int i = 0; i < n; ++i)
        it->second->append (static_cast<uint8_t> (e[i]));
    return true;
}

void Sysex7Builder::completeMessage (const Sysex7Event& e)
{
    const int groupKey = e.group.get ();
    auto it            = inProgressBuffers.find (groupKey);
    if (it == inProgressBuffers.end ())
        return;

    Sysex7Message msg { MidiNibble { groupKey }, it->second };
    msg.timestamp     = e.timestamp.get ();
    msg.endpointIndex = e.endpointIndex.get ();
    msg.endpointName  = e.endpointName.get ();
    msg.isReceived    = e.isReceived.get ();
    inProgressBuffers.erase (it);

    eventList.addMessage (msg);
}

#if RUN_UNIT_TESTS
#include "test/test_Sysex7Builder.inl"
#endif
