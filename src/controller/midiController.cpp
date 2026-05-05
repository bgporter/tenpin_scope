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

#include "endpointController.h"
#include "model/ump/channelVoice2.h"
#if JUCE_DEBUG
#include "syntheticEndpointController.h"
#endif
#include "model/ump/umpEvent.h"
#include "utility/logger.h"

MidiController::MidiController (juce::StringRef sessionName, AppContext& appContext)
: persistentContext { appContext }
, runtimeContext { appContext }
, midiProperties { runtimeContext }
, eventFilter { appContext }
, endpoints { juce::ump::Endpoints::getInstance () }
, session { endpoints->makeSession (sessionName) }
{
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

    // !!! listen for new endpoints added to the endpoints object
    midiProperties.endpoints.onChildAdded = [this] (juce::ValueTree& vt, int _, int index)
    {
        // we've added a new endpointProperties child to the midiProperties object.
        // we want to add listeners to the sent/received lists.
        auto thisEndpointProperties = std::make_unique<MidiEndpointProperties> (vt);
        const auto propIndex        = endpointProperties.size ();

        thisEndpointProperties->received.onChildAdded = [this, propIndex] (juce::ValueTree& vt, int, int)
        {
            UmpEvent event (vt.createCopy ());

            // const auto endpointProps { endpointProperties[propIndex].get () };
            // event.endpointName = endpointProps->name.get ();
            // event.isReceived   = true;
            addMidiEvent (event);
        };
        thisEndpointProperties->received.onChildRemoved = [this, propIndex] (juce::ValueTree& vt, int, int)
        { DBG ("REMOVING EVENT FROM " << endpointProperties[propIndex]->name); };
        // TEMPORARY: clear this endpoint's received list when count hits 100, 200, etc.
        thisEndpointProperties->received.count.onPropertyChange (
            [this, propIndex] (const juce::Identifier&)
            {
                const int c = endpointProperties[propIndex]->received.count.get ();
                if (c > 0 && c % 300 == 0)
                {
                    DBG ("&&&&& CLEARING EVENTS FrOM " << endpointProperties[propIndex]->name);
                    endpointProperties[propIndex]->received.clear ();
                    rebuildMidiEventsFromEndpoints ();
                }
            });
        thisEndpointProperties->transmitted.onChildAdded = [this, propIndex] (juce::ValueTree& vt, int, int)
        {
            UmpEvent event (vt);
            const auto endpointProps { endpointProperties[propIndex].get () };
            event.endpointName = endpointProps->name.get ();
            event.isReceived   = false;
            addMidiEvent (event);
        };
        endpointProperties.push_back (std::move (thisEndpointProperties));
    };

    int i = 0;
    for (const auto& endpointId : endpoints->getEndpoints ())
    {
        if (auto endpoint = endpoints->getEndpoint (endpointId))
        {
            addEndpointController (i, endpointId);
            ++i;
        }
    }
    INFO_ ({
        {   "msg", "Found MIDI endpoints" },
        { "count",                      i }
    });

    endpoints->addListener (*this);
    startTimer (33); // 30 times per second (30 Hz)
#if JUCE_DEBUG
    syntheticController = std::make_unique<SyntheticEndpointController> (midiProperties, "Synthetic Input");
#endif

    persistentContext.eventViewContext.onPropertyChange ([this] (const juce::Identifier&)
                                                         { rebuildMidiEventsFromEndpoints (); });
}

MidiController::~MidiController ()
{
    stopTimer ();
    endpoints->removeListener (*this);
    endpointControllers.clear ();
}

void MidiController::endpointsChanged ()
{
    INFO_ ("Endpoints changed");

    // posibilities:
    // an endpoint was disconnected
    const auto availableEndpoints = endpoints->getEndpoints ();
    for (const auto& endpointController : endpointControllers)
    {
        // any endpoints IDs that we know about but are no longer present in the
        // vector of connected endpoints has been disconnected. We set it
        // as such but do NOT delete it.
        if (std::find_if (availableEndpoints.begin (), availableEndpoints.end (), [&] (const auto& endpointId)
                          { return endpointController->getEndpointId () == endpointId; }) == availableEndpoints.end ())
            endpointController->disconnected ();
    }
    // an endpoint was added or otherwise modified.
    for (const auto& endpointId : availableEndpoints)
    {
        if (auto ec = std::find_if (endpointControllers.begin (), endpointControllers.end (),
                                    [&] (const auto& endpointController)
                                    { return endpointController->getEndpointId () == endpointId; });
            ec != endpointControllers.end ())
            updateEndpointController (endpointId);
        else
            addEndpointController (static_cast<int> (endpointControllers.size ()), endpointId);
    }
}

void MidiController::addEndpointController (int index, juce::ump::EndpointId endpointId)
{
    DEBUG_ ("Adding endpoint controller");
    jassert (juce::MessageManager::getInstance ()->isThisTheMessageThread ());
    auto endpoint = endpoints->getEndpoint (endpointId);
    INFO_ ({
        {   "msg",                          "Device found" },
        {  "name",                    endpoint->getName () },
        { "srcId", endpointId.get (juce::ump::IOKind::src) },
        { "dstId", endpointId.get (juce::ump::IOKind::dst) }
    });
    auto endpointController = std::make_unique<EndpointController> (index, endpointId, midiProperties);
    endpointController->connectEndpoint (&session);

    // we keep track of two things:
    // 1. the EndpointController object (which only we here inside the MidiController
    //    have direct access to)
    // 2. the MidiEndpointProperties object (which we use to track the state of the
    //    endpoint and is shared with the rest of the app, available via the RuntimeContext/MidiProperties object)
    endpointControllers.push_back (std::move (endpointController));
}

void MidiController::updateEndpointController (juce::ump::EndpointId endpointId)
{
    // 1. find the endpoint controller in our list of endpoint controllers
    auto endpointController =
        std::find_if (endpointControllers.begin (), endpointControllers.end (),
                      [&] (const auto& controller) { return controller->getEndpointId () == endpointId; });
    if (endpointController == endpointControllers.end ())
    {
        ERROR_ ({
            { "msg",                                                         "Endpoint controller not found" },
            {  "id", endpointId.get (juce::ump::IOKind::src) + "-" + endpointId.get (juce::ump::IOKind::dst) }
        });
        return;
    }
    // 2. update the endpoint controller
    (*endpointController)->connectEndpoint (&session);
}

void MidiController::timerCallback ()
{
    for (auto& endpointController : endpointControllers)
        endpointController->processUmpEvents ();
#if JUCE_DEBUG
    if (syntheticController)
        syntheticController->processUmpEvents ();
#endif
}

void MidiController::addMidiEvent (UmpEvent& event)
{
    if (eventFilter.filterMidiEvent (event))
        midiProperties.midiEvents.addEvent (event);
}

void MidiController::rebuildMidiEventsFromEndpoints ()
{
    midiProperties.midiEvents.isRebuilding = true;
    // concatenate all the events from all the endpoints into a single event list
    EventList currentEvents { "midiEvents" };
    for (const auto& ep : endpointProperties)
    {
        DBG ("REBUILDING FROM " << ep->name << " has " << ep->received.getNumChildren ());
        int i { 0 };
        for (; i < ep->received.getNumChildren (); ++i)
        {
            // auto ev = ep->received[i].createCopy ();
            // DBG (ev.toXmlString ());
            UmpEvent evCopy (ep->received[i].createCopy ());
            if (eventFilter.filterMidiEvent (evCopy))
                currentEvents.addEvent (evCopy);
        }
        DBG ("Added " << i << " RX events from  " << ep->name);
        for (i = 0; i < ep->transmitted.getNumChildren (); ++i)
        {
            UmpEvent evCopy (ep->transmitted[i].createCopy ());
            if (eventFilter.filterMidiEvent (evCopy))
                currentEvents.addEvent (evCopy);
        }
        DBG ("Added " << ep->transmitted.count.get () << " TX events from  " << ep->name);
    }
    // sort the events by timestamp
    cello::Query sorter { currentEvents.getType () };
    sorter.addComparison (
        [] (const juce::ValueTree& a, const juce::ValueTree& b)
        { return a.getProperty (UmpEvent::timestampId, 0.0) < b.getProperty (UmpEvent::timestampId, 0.0) ? -1 : 1; });
    sorter.sort (currentEvents);

    // add the events to the midiProperties.midiEvents list
    midiProperties.midiEvents.clear ();
    int i { 0 };
    for (; i < currentEvents.getNumChildren (); ++i)
    {
        juce::ValueTree ev = currentEvents[i].createCopy ();
        jassert (ev.isValid ());
        UmpEvent evCopy (ev);
        midiProperties.midiEvents.addEvent (evCopy);
    }
    DBG ("rebuilt midiEvents from endpoints: " << i << " events");
    midiProperties.midiEvents.isRebuilding = false;
}
