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

#pragma once

#include <JuceHeader.h>
#include <functional>
#include <map>

#include "eventView.h"
#include "handler/umpHandler.h"
#include "model/appContext.h"
#include "model/eventList.h"
#include "model/midiProperties.h"
#include "model/runtimeContext.h"
#include "model/ump/channelVoice2.h"

enum class EventListChange
{
    appended,
    cleared
};

class EventListView : public juce::Component
{
public:
    using OnEventListChangedFn = std::function<void (EventListChange change, int newCount)>;
    using OnBeforeEventAddedFn = std::function<void ()>;

    EventListView (AppContext& theAppContext, OnEventListChangedFn onEventListChanged = nullptr,
                   OnBeforeEventAddedFn onBeforeEventAdded = nullptr);

    void paint (juce::Graphics& g) override;

    void resized () override;

    /**
     * @brief respond to resize events affecting the viewport that owns this component.
     */
    void parentSizeChanged () override;

    /**
     * @brief Calculate the sum of the heights of our contained components.
     *
     * @return int total required height.
     */
    int getContentHeight () const;

    /**
     * @brief Called when viewport scrolls to update visible views
     */
    void visibleAreaChanged (const juce::Rectangle<int>& newVisibleArea);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EventListView)

    void handleEventAdded (juce::ValueTree& vt);
    void handleChildrenCleared ();
    int findEventAtYPosition (int yPos) const;
    void cullViewsOutsideRange (const juce::Range<int>& keepRange);
    void createViewsInRange (const juce::Range<int>& range);
    void positionView (int index);
    void recalculateCumulativeHeightsFrom (int startIndex);
    void updateContentSize ();
    bool isInBufferZone (int eventIndex) const;

    AppContext appContext;
    RuntimeContext runtimeContext;
    MidiProperties midiProperties;
    EventList eventList;
    OnEventListChangedFn onEventListChanged;
    OnBeforeEventAddedFn onBeforeEventAdded;

    std::unique_ptr<class EventListViewHandler> handler;

    // Height metadata (persistent for ALL events)
    std::vector<int> eventHeights;      // One height per event
    std::vector<int> cumulativeHeights; // Prefix sums for O(1) position lookup

    // Active views (sparse, only visible + buffer)
    std::map<int, std::unique_ptr<EventView>> activeViews; // Index → View

    // Viewport tracking
    int bufferZoneEvents { 100 };         // Keep ±100 events outside viewport
    juce::Range<int> currentVisibleRange; // Currently visible event indices
};

class EventListViewHandler : public UmpHandler
{
public:
    EventListViewHandler (AppContext& theAppContext)
    : appContext (theAppContext)
    {
    }
    ~EventListViewHandler () override { eventView.reset (); }

    std::unique_ptr<EventView> getEventView () { return std::move (eventView); }

private:
    AppContext appContext;
    juce::String eventDescription;
    std::unique_ptr<EventView> eventView;

    UmpHandler::Result preDispatch (const UmpEvent& event) override
    {
        eventView.reset ();
        eventDescription.clear ();
        eventDescription << event.timestamp << " ";
        eventDescription << event.endpointName << " ";
        eventDescription << (event.isReceived ? "Rx" : "Tx") << " ";
        return Result::ok;
    }

    UmpHandler::Result postDispatch (UmpHandler::Result pendingResult) override
    {
        if (pendingResult == UmpHandler::Result::ok)
            eventView = std::make_unique<EventView> (appContext, eventDescription);
        eventDescription.clear ();
        return pendingResult;
    }

    UmpHandler::Result onMidi2NoteOffEvent (const UmpEvent& event) override
    {
        Midi2NoteOffEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Note Off";
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2NoteOnEvent (const UmpEvent& event) override
    {
        Midi2NoteOnEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Note On";
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2NoteEvent (const UmpEvent& event) override
    {
        Midi2NoteEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel << " -- Note";
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2PerNoteEvent (const UmpEvent& event) override
    {
        Midi2PerNoteEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Per Note";
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2PerNotePitchBendEvent (const UmpEvent& event) override
    {
        Midi2PerNotePitchBendEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Per Note Pitch Bend";
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2ControlChangeEvent (const UmpEvent& event) override
    {
        Midi2ControlChangeEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Control Change";
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2ProgramChangeEvent (const UmpEvent& event) override
    {
        Midi2ProgramChangeEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Program Change";
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2PerNoteManagementEvent (const UmpEvent& event) override
    {
        Midi2PerNoteManagementEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Per Note Management";
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2PolyPressureEvent (const UmpEvent& event) override
    {
        Midi2PolyPressureEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Poly Pressure";
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2ChannelPressureEvent (const UmpEvent& event) override
    {
        Midi2ChannelPressureEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Channel Pressure";
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2PitchBendEvent (const UmpEvent& event) override
    {
        Midi2PitchBendEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Pitch Bend";
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2RegisteredPerNoteControllerEvent (const UmpEvent& event) override
    {
        Midi2RegisteredPerNoteControllerEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Registered Per Note Controller";
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2AssignablePerNoteControllerEvent (const UmpEvent& event) override
    {
        Midi2AssignablePerNoteControllerEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Assignable Per Note Controller";
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2ControllerEvent (const UmpEvent& event) override
    {
        Midi2ControllerEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Controller";
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2RegisteredControllerEvent (const UmpEvent& event) override
    {
        Midi2RegisteredControllerEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Registered Controller";
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2AssignableControllerEvent (const UmpEvent& event) override
    {
        Midi2AssignableControllerEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Assignable Controller";
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2RelativeControllerEvent (const UmpEvent& event) override
    {
        Midi2RelativeControllerEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Relative Controller";
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2RelativeRegisteredControllerEvent (const UmpEvent& event) override
    {
        Midi2RelativeRegisteredControllerEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Relative Registered Controller";
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2RelativeAssignableControllerEvent (const UmpEvent& event) override
    {
        Midi2RelativeAssignableControllerEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Relative Assignable Controller";
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2ChannelVoiceEvent (const UmpEvent& event) override
    {
        Midi2ChannelVoiceEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- MIDI2 Channel Voice";
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onUmpEvent (const UmpEvent& event) override
    {
        eventDescription << "UMP DATA: ";
        eventDescription << juce::String::formatted ("%08X ", event.getattr<uint32_t> (UmpWords::data0Id, 0));
        eventDescription << juce::String::formatted ("%08X", event.getattr<uint32_t> (UmpWords::data1Id, 0));
        return UmpHandler::Result::ok;
    }
};
