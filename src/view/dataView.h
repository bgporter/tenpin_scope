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

#include "eventListView.h"
#include "handler/umpHandler.h"
#include "model/appContext.h"
#include "model/midiEndpointProperties.h"
#include "model/midiProperties.h"
#include "model/runtimeContext.h"
#include "model/ump/channelVoice2.h"

class DataViewHandler;
class EventView;

class DataView : public juce::Component,
                 public juce::ScrollBar::Listener
{
public:
    DataView (AppContext& appContext);
    ~DataView () override;

    void paint (juce::Graphics& g) override;

    void resized () override;

    void addEndpoint (juce::ValueTree vt, int index);
    void removeEndpoint (juce::ValueTree vt, int index);
    void addEventView (std::unique_ptr<EventView> eventView);

    // ScrollBar::Listener
    void scrollBarMoved (juce::ScrollBar* scrollBar, double newRangeStart) override;

private:
    AppContext appContext;
    RuntimeContext runtimeContext;
    MidiProperties midiProperties;
    std::unique_ptr<DataViewHandler> dataViewHandler;
    std::vector<std::unique_ptr<MidiEndpointProperties>> endpoints;
    juce::Viewport viewport;
    EventListView eventListView;
};

class DataViewHandler : public UmpHandler
{
public:
    DataViewHandler (DataView& theDataView, AppContext& theAppContext)
    : appContext (theAppContext)
    , dataView (theDataView)
    {
    }
    ~DataViewHandler () override {}

private:
    AppContext appContext;
    DataView& dataView;
    juce::String eventDescription;

    UmpHandler::Result preDispatch (const UmpEvent& event) override
    {
        eventDescription.clear ();
        eventDescription << event.timestamp << " ";
        return Result::ok;
    }

    UmpHandler::Result postDispatch (UmpHandler::Result pendingResult) override
    {
        if (pendingResult == UmpHandler::Result::ok)
        {
            dataView.addEventView (std::make_unique<EventView> (appContext, eventDescription));
        }
        eventDescription.clear ();
        return pendingResult;
    }

    UmpHandler::Result onMidi2NoteOffEvent (const UmpEvent& event) override
    {
        Midi2NoteOffEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Note Off";
        DBG (eventDescription);
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2NoteOnEvent (const UmpEvent& event) override
    {
        Midi2NoteOnEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Note On";
        DBG (eventDescription);
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2NoteEvent (const UmpEvent& event) override
    {
        Midi2NoteEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel << " -- Note";
        DBG (eventDescription);
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2PerNoteEvent (const UmpEvent& event) override
    {
        Midi2PerNoteEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Per Note";
        DBG (eventDescription);
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2PerNotePitchBendEvent (const UmpEvent& event) override
    {
        Midi2PerNotePitchBendEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Per Note Pitch Bend";
        DBG (eventDescription);
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2ControlChangeEvent (const UmpEvent& event) override
    {
        Midi2ControlChangeEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Control Change";
        DBG (eventDescription);
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2ProgramChangeEvent (const UmpEvent& event) override
    {
        Midi2ProgramChangeEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Program Change";
        DBG (eventDescription);
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2PerNoteManagementEvent (const UmpEvent& event) override
    {
        Midi2PerNoteManagementEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Per Note Management";
        DBG (eventDescription);
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2PolyPressureEvent (const UmpEvent& event) override
    {
        Midi2PolyPressureEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Poly Pressure";
        DBG (eventDescription);
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2ChannelPressureEvent (const UmpEvent& event) override
    {
        Midi2ChannelPressureEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Channel Pressure";
        DBG (eventDescription);
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2PitchBendEvent (const UmpEvent& event) override
    {
        Midi2PitchBendEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Pitch Bend";
        DBG (eventDescription);
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2RegisteredPerNoteControllerEvent (const UmpEvent& event) override
    {
        Midi2RegisteredPerNoteControllerEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Registered Per Note Controller";
        DBG (eventDescription);
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2AssignablePerNoteControllerEvent (const UmpEvent& event) override
    {
        Midi2AssignablePerNoteControllerEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Assignable Per Note Controller";
        DBG (eventDescription);
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2ControllerEvent (const UmpEvent& event) override
    {
        Midi2ControllerEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Controller";
        DBG (eventDescription);
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2RegisteredControllerEvent (const UmpEvent& event) override
    {
        Midi2RegisteredControllerEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Registered Controller";
        DBG (eventDescription);
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2AssignableControllerEvent (const UmpEvent& event) override
    {
        Midi2AssignableControllerEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Assignable Controller";
        DBG (eventDescription);
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2RelativeControllerEvent (const UmpEvent& event) override
    {
        Midi2RelativeControllerEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Relative Controller";
        DBG (eventDescription);
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2RelativeRegisteredControllerEvent (const UmpEvent& event) override
    {
        Midi2RelativeRegisteredControllerEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Relative Registered Controller";
        DBG (eventDescription);
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2RelativeAssignableControllerEvent (const UmpEvent& event) override
    {
        Midi2RelativeAssignableControllerEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- Relative Assignable Controller";
        DBG (eventDescription);
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onMidi2ChannelVoiceEvent (const UmpEvent& event) override
    {
        Midi2ChannelVoiceEvent midi2Event (event);
        eventDescription << "Group: " << midi2Event.userGroup << " Channel: " << midi2Event.userChannel
                         << " -- MIDI2 Channel Voice";
        DBG (eventDescription);
        return UmpHandler::Result::ok;
    }

    UmpHandler::Result onUmpEvent (const UmpEvent& event) override
    {
        eventDescription << "UMP DATA: ";
        eventDescription << juce::String::formatted ("%08X ", event.getattr<uint32_t> (UmpWords::data0Id, 0));
        eventDescription << juce::String::formatted ("%08X", event.getattr<uint32_t> (UmpWords::data1Id, 0));
        DBG (eventDescription);
        return UmpHandler::Result::ok;
    }
};
