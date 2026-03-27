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

#pragma once

#include <JuceHeader.h>
#include <functional>
#include <map>

#include "eventNameUtils.h"
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
    EventListView (AppContext& theAppContext);

    void paint (juce::Graphics& g) override;

    void resized () override;

    /**
     * @brief Removes all EventViews from the list/display, returning them to the pool
     * and clears the event positions.
     *
     */
    void clear ();

    /**
     * @brief Called when viewport scrolls to update visible views.
     * We need to:
     * - map from the visible area to the event indices
     * - remove any views that are no longer needed
     * - create any views that are needed, and add them to the visible event views
     * - position the views
     */
    void visibleAreaChanged (const juce::Rectangle<int>& newVisibleArea);

    /**
     * @brief Called when the width of the list changes, so we can recalculate the positions of the events.
     */
    void widthChanged (int newWidth);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EventListView)

    int findEventAtYPosition (int yPos) const;
    void createViewsInRange (const juce::Range<int>& range);

    /**
     * @brief Called when the parent hierarchy changes, so we can find the
     * viewport that owns us.
     */
    void parentHierarchyChanged () override
    {
        if (viewport == nullptr)
            viewport = findParentComponentOfClass<juce::Viewport> ();
    }

    /**
     * @brief Determine if the new event should be displayed.
     */
    bool shouldDisplayNewEvent () const;
    void addEvent (juce::ValueTree vt);
    enum class InsertionPoint
    {
        top,
        bottom
    };

    void displayEvent (std::unique_ptr<EventView> eventView, int index, InsertionPoint insertionPoint);
    void hideEvent (InsertionPoint insertionPoint);

    std::unique_ptr<EventView> createEventView (juce::ValueTree vt, int index, int width);
    class EventViewPool
    {
    public:
        EventViewPool (AppContext& theAppContext)
        : appContext (theAppContext)
        {
        }
        ~EventViewPool () = default;

        std::unique_ptr<EventView> getEventView ()
        {
            if (pool.empty ())
            {
                DBG ("EventViewPool: allocating (pool empty)");
                return std::make_unique<EventView> (appContext);
            }
            DBG ("EventViewPool: reusing from pool (size=" << pool.size () << ")");
            auto eventView = std::move (pool.top ());
            pool.pop ();
            return eventView;
        }

        void returnEventView (std::unique_ptr<EventView> view)
        {
            view->reset ();
            DBG ("EventViewPool: returning to pool (size now=" << (pool.size () + 1) << ")");
            pool.push (std::move (view));
        }

    private:
        std::stack<std::unique_ptr<EventView>> pool;
        AppContext appContext;
    };

    AppContext appContext;
    RuntimeContext runtimeContext;
    MidiProperties midiProperties;
    EventList eventList;

    juce::Viewport* viewport { nullptr };
    std::unique_ptr<class EventListViewHandler> handler;
    EventViewPool eventViewPool;
    std::deque<std::unique_ptr<EventView>> visibleEventViews;
    juce::Range<int> visibleEventRange;
    juce::Rectangle<int> visibleArea;

    // upper-left y-position metadata (persistent for ALL events),
    // recalculated as needed.
    std::vector<int> eventPositions;

    // Guard against visibleAreaChanged firing mid-rebuild in widthChanged
    bool isRecalculating { false };
};

class EventListViewHandler : public UmpHandler
{
public:
    EventListViewHandler (AppContext& theAppContext)
    : appContext (theAppContext)
    {
    }
    ~EventListViewHandler () override { eventView = nullptr; }

    UmpHandler::Result handle (const UmpEvent& event, int index, EventView* view, int width)
    {
        eventView    = view;
        currentIndex = index;
        currentWidth = width;
        return UmpHandler::handle (event);
    }

private:
    UmpHandler::Result preDispatch (const UmpEvent& event) override
    {
        const juce::String timeStr = juce::String (currentIndex) + ": " + juce::String ((double) event.timestamp, 3);
        eventView->setTime (timeStr);

        const juce::String endpointStr = juce::String (event.endpointName) + " " + (event.isReceived ? "Rx" : "Tx");
        eventView->setEndpoint (endpointStr);

        return Result::ok;
    }

    UmpHandler::Result postDispatch (UmpHandler::Result pendingResult) override
    {
        if (pendingResult == UmpHandler::Result::ok)
            eventView->sizeToWidth (currentWidth);

        eventView    = nullptr;
        currentIndex = -1;
        currentWidth = 0;
        return pendingResult;
    }

    UmpHandler::Result onMidi2NoteOffEvent (const UmpEvent& event) override
    {
        Midi2NoteOffEvent e (event);
        eventView->addValue ("MIDI 2 Note Off", "");
        eventView->addValue ("grp", juce::String ((int) e.userGroup));
        eventView->addValue ("ch", juce::String ((int) e.userChannel));
        eventView->addValue ("note", getNoteName ((int) e.note));
        eventView->addValue ("vel", juce::String ((int) e.velocity));
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2NoteOnEvent (const UmpEvent& event) override
    {
        Midi2NoteOnEvent e (event);
        eventView->addValue ("MIDI 2 Note On", "");
        eventView->addValue ("grp", juce::String ((int) e.userGroup));
        eventView->addValue ("ch", juce::String ((int) e.userChannel));
        eventView->addValue ("note", getNoteName ((int) e.note));
        eventView->addValue ("vel", juce::String ((int) e.velocity));
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2NoteEvent (const UmpEvent& event) override
    {
        Midi2NoteEvent e (event);
        eventView->addValue ("MIDI 2 Note", "");
        eventView->addValue ("grp", juce::String ((int) e.userGroup));
        eventView->addValue ("ch", juce::String ((int) e.userChannel));
        eventView->addValue ("note", getNoteName ((int) e.note));
        eventView->addValue ("vel", juce::String ((int) e.velocity));
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2RegisteredPerNoteControllerEvent (const UmpEvent& event) override
    {
        Midi2RegisteredPerNoteControllerEvent e (event);
        eventView->addValue ("MIDI 2 Registered Per Note Controller", "");
        eventView->addValue ("grp", juce::String ((int) e.userGroup));
        eventView->addValue ("ch", juce::String ((int) e.userChannel));
        eventView->addValue ("note", getNoteName ((int) e.note));
        eventView->addValue ("ctrl", getControllerName ((int) e.controller));
        eventView->addValue ("val", juce::String ((float) e.valueFloat, 3));
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2AssignablePerNoteControllerEvent (const UmpEvent& event) override
    {
        Midi2AssignablePerNoteControllerEvent e (event);
        eventView->addValue ("MIDI 2 Assignable Per Note Controller", "");
        eventView->addValue ("grp", juce::String ((int) e.userGroup));
        eventView->addValue ("ch", juce::String ((int) e.userChannel));
        eventView->addValue ("note", getNoteName ((int) e.note));
        eventView->addValue ("ctrl", getControllerName ((int) e.controller));
        eventView->addValue ("val", juce::String ((float) e.valueFloat, 3));
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2PerNoteEvent (const UmpEvent& event) override
    {
        Midi2PerNoteEvent e (event);
        eventView->addValue ("MIDI 2 Per Note", "");
        eventView->addValue ("grp", juce::String ((int) e.userGroup));
        eventView->addValue ("ch", juce::String ((int) e.userChannel));
        eventView->addValue ("note", getNoteName ((int) e.note));
        eventView->addValue ("ctrl", getControllerName ((int) e.controller));
        eventView->addValue ("val", juce::String ((float) e.valueFloat, 3));
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2PerNotePitchBendEvent (const UmpEvent& event) override
    {
        Midi2PerNotePitchBendEvent e (event);
        eventView->addValue ("MIDI 2 Per Note Pitch Bend", "");
        eventView->addValue ("grp", juce::String ((int) e.userGroup));
        eventView->addValue ("ch", juce::String ((int) e.userChannel));
        eventView->addValue ("note", getNoteName ((int) e.note));
        eventView->addValue ("val", juce::String ((float) e.valueFloat, 3));
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2ControlChangeEvent (const UmpEvent& event) override
    {
        Midi2ControlChangeEvent e (event);
        eventView->addValue ("MIDI 2 Control Change", "");
        eventView->addValue ("grp", juce::String ((int) e.userGroup));
        eventView->addValue ("ch", juce::String ((int) e.userChannel));
        eventView->addValue ("ctrl", getControllerName ((int) e.controller));
        eventView->addValue ("val", juce::String ((float) e.valueFloat, 3));
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2ProgramChangeEvent (const UmpEvent& event) override
    {
        Midi2ProgramChangeEvent e (event);
        eventView->addValue ("MIDI 2 Program Change", "");
        eventView->addValue ("grp", juce::String ((int) e.userGroup));
        eventView->addValue ("ch", juce::String ((int) e.userChannel));
        eventView->addValue ("prog", juce::String ((int) e.program));
        if ((bool) e.bankValid)
            eventView->addValue ("bank", juce::String ((int) e.bank));
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2PerNoteManagementEvent (const UmpEvent& event) override
    {
        Midi2PerNoteManagementEvent e (event);
        eventView->addValue ("MIDI 2 Per Note Management", "");
        eventView->addValue ("grp", juce::String ((int) e.userGroup));
        eventView->addValue ("ch", juce::String ((int) e.userChannel));
        eventView->addValue ("note", getNoteName ((int) e.note));
        eventView->addValue ("detach", (bool) e.detach ? "Y" : "N");
        eventView->addValue ("reset", (bool) e.reset ? "Y" : "N");
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2PolyPressureEvent (const UmpEvent& event) override
    {
        Midi2PolyPressureEvent e (event);
        eventView->addValue ("MIDI 2 Poly Pressure", "");
        eventView->addValue ("grp", juce::String ((int) e.userGroup));
        eventView->addValue ("ch", juce::String ((int) e.userChannel));
        eventView->addValue ("note", getNoteName ((int) e.note));
        eventView->addValue ("val", juce::String ((float) e.valueFloat, 3));
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2ChannelPressureEvent (const UmpEvent& event) override
    {
        Midi2ChannelPressureEvent e (event);
        eventView->addValue ("MIDI 2 Channel Pressure", "");
        eventView->addValue ("grp", juce::String ((int) e.userGroup));
        eventView->addValue ("ch", juce::String ((int) e.userChannel));
        eventView->addValue ("val", juce::String ((float) e.valueFloat, 3));
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2PitchBendEvent (const UmpEvent& event) override
    {
        Midi2PitchBendEvent e (event);
        eventView->addValue ("MIDI 2 Pitch Bend", "");
        eventView->addValue ("grp", juce::String ((int) e.userGroup));
        eventView->addValue ("ch", juce::String ((int) e.userChannel));
        eventView->addValue ("val", juce::String ((float) e.valueFloat, 3));
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2RegisteredControllerEvent (const UmpEvent& event) override
    {
        Midi2RegisteredControllerEvent e (event);
        eventView->addValue ("MIDI 2 Registered Controller", "");
        eventView->addValue ("grp", juce::String ((int) e.userGroup));
        eventView->addValue ("ch", juce::String ((int) e.userChannel));
        eventView->addValue ("bank", juce::String ((int) e.bank));
        eventView->addValue ("ctrl", juce::String ((int) e.controller));
        eventView->addValue ("val", juce::String ((float) e.valueFloat, 3));
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2AssignableControllerEvent (const UmpEvent& event) override
    {
        Midi2AssignableControllerEvent e (event);
        eventView->addValue ("MIDI 2 Assignable Controller", "");
        eventView->addValue ("grp", juce::String ((int) e.userGroup));
        eventView->addValue ("ch", juce::String ((int) e.userChannel));
        eventView->addValue ("bank", juce::String ((int) e.bank));
        eventView->addValue ("ctrl", juce::String ((int) e.controller));
        eventView->addValue ("val", juce::String ((float) e.valueFloat, 3));
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2ControllerEvent (const UmpEvent& event) override
    {
        Midi2ControllerEvent e (event);
        eventView->addValue ("MIDI 2 Control Change", "");
        eventView->addValue ("grp", juce::String ((int) e.userGroup));
        eventView->addValue ("ch", juce::String ((int) e.userChannel));
        eventView->addValue ("bank", juce::String ((int) e.bank));
        eventView->addValue ("ctrl", juce::String ((int) e.controller));
        eventView->addValue ("val", juce::String ((float) e.valueFloat, 3));
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2RelativeRegisteredControllerEvent (const UmpEvent& event) override
    {
        Midi2RelativeRegisteredControllerEvent e (event);
        eventView->addValue ("MIDI 2 Relative Registered Controller", "");
        eventView->addValue ("grp", juce::String ((int) e.userGroup));
        eventView->addValue ("ch", juce::String ((int) e.userChannel));
        eventView->addValue ("bank", juce::String ((int) e.bank));
        eventView->addValue ("ctrl", juce::String ((int) e.controller));
        eventView->addValue ("val", juce::String ((float) e.valueFloat, 3));
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2RelativeAssignableControllerEvent (const UmpEvent& event) override
    {
        Midi2RelativeAssignableControllerEvent e (event);
        eventView->addValue ("MIDI 2 Relative Assignable Controller", "");
        eventView->addValue ("grp", juce::String ((int) e.userGroup));
        eventView->addValue ("ch", juce::String ((int) e.userChannel));
        eventView->addValue ("bank", juce::String ((int) e.bank));
        eventView->addValue ("ctrl", juce::String ((int) e.controller));
        eventView->addValue ("val", juce::String ((float) e.valueFloat, 3));
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2RelativeControllerEvent (const UmpEvent& event) override
    {
        Midi2RelativeControllerEvent e (event);
        eventView->addValue ("MIDI 2 Relative Control Change", "");
        eventView->addValue ("grp", juce::String ((int) e.userGroup));
        eventView->addValue ("ch", juce::String ((int) e.userChannel));
        eventView->addValue ("bank", juce::String ((int) e.bank));
        eventView->addValue ("ctrl", juce::String ((int) e.controller));
        eventView->addValue ("val", juce::String ((float) e.valueFloat, 3));
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2ChannelVoiceEvent (const UmpEvent& event) override
    {
        Midi2ChannelVoiceEvent e (event);
        eventView->addValue ("MIDI 2 Channel Voice", "");
        eventView->addValue ("grp", juce::String ((int) e.userGroup));
        eventView->addValue ("ch", juce::String ((int) e.userChannel));
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onUmpEvent (const UmpEvent& event) override
    {
        eventView->addValue ("MIDI 2 UMP", "");
        eventView->addValue ("d0", juce::String::formatted ("%08X", event.getattr<uint32_t> (UmpWords::data0Id, 0)));
        eventView->addValue ("d1", juce::String::formatted ("%08X", event.getattr<uint32_t> (UmpWords::data1Id, 0)));
        return UmpHandler::Result::ok;
    }

    AppContext appContext;
    EventView* eventView { nullptr };
    int currentIndex { -1 };
    int currentWidth { 0 };
};
