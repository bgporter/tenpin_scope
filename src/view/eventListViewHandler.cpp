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

#include "eventListViewHandler.h"

#include "model/ump/channelVoice2.h"
#include "palette.h"

EventListViewHandler::EventListViewHandler (AppContext& theAppContext)
: appContext (theAppContext)
, pc { theAppContext }
{
}

EventListViewHandler::~EventListViewHandler ()
{
    eventView = nullptr;
}

UmpHandler::Result EventListViewHandler::handle (const UmpEvent& event, int index, EventView* view, int width)
{
    eventView = view;
    Palette pal { pc };

    eventView->setColors (pal.umpBackground.get (), pal.midi2Label.get (), pal.midi2Value.get (), pal.outline.get ());

    currentIndex = index;
    currentWidth = width;
    return UmpHandler::handle (event);
}

UmpHandler::Result EventListViewHandler::preDispatch (const UmpEvent& event)
{
    const auto timeStr = juce::String ((double) event.timestamp, 3);
    eventView->setTime (timeStr);

    const auto endpointStr = juce::String (event.endpointName) + " " + (event.isReceived ? "Rx" : "Tx");
    eventView->setEndpoint (endpointStr);

    return Result::ok;
}

UmpHandler::Result EventListViewHandler::postDispatch (const UmpEvent& event, UmpHandler::Result pendingResult)
{
    if (pendingResult == UmpHandler::Result::ok)
    {
        if (pc.eventViewContext.umpShowRawData)
        {
            eventView->addLine ();
            // !!! actually get and format the correct hex data.
            eventView->addValue (
                "raw", formatValue (event.getattr<uint32_t> (UmpWords::data0Id, 0), 32, ValueFormatType::Hex));
            if (event.hasattr (UmpWords::data1Id))
            {
                eventView->addValue (
                    "", formatValue (event.getattr<uint32_t> (UmpWords::data1Id, 0), 32, ValueFormatType::Hex));
                if (event.hasattr (UmpWords::data2Id))
                {
                    eventView->addValue (
                        "", formatValue (event.getattr<uint32_t> (UmpWords::data2Id, 0), 32, ValueFormatType::Hex));
                    if (event.hasattr (UmpWords::data3Id))
                    {
                        eventView->addValue (
                            "", formatValue (event.getattr<uint32_t> (UmpWords::data3Id, 0), 32, ValueFormatType::Hex));
                    }
                }
            }
        }
        eventView->sizeToWidth (currentWidth);
    }

    eventView    = nullptr;
    currentIndex = -1;
    currentWidth = 0;
    return pendingResult;
}

UmpHandler::Result EventListViewHandler::onMidi2NoteOffEvent (const UmpEvent& event)
{
    Midi2NoteOffEvent e (event);
    eventView->setEvent ("MIDI 2 Note Off");
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("note", getNoteName ((int) e.note, pc.eventViewContext.octaveType));
    eventView->addValue ("vel", formatValue ((uint32_t) e.velocity, 16, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2NoteOnEvent (const UmpEvent& event)
{
    Midi2NoteOnEvent e (event);
    eventView->setEvent ("MIDI 2 Note On");
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("note", getNoteName ((int) e.note, pc.eventViewContext.octaveType));
    eventView->addValue ("vel", formatValue ((uint32_t) e.velocity, 16, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2NoteEvent (const UmpEvent& event)
{
    Midi2NoteEvent e (event);
    eventView->setEvent ("MIDI 2 Note");
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("note", getNoteName ((int) e.note, pc.eventViewContext.octaveType));
    eventView->addValue ("vel", formatValue ((uint32_t) e.velocity, 16, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2RegisteredPerNoteControllerEvent (const UmpEvent& event)
{
    Midi2RegisteredPerNoteControllerEvent e (event);
    eventView->setEvent ("MIDI 2 Registered Per Note Controller");
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("note", getNoteName ((int) e.note, pc.eventViewContext.octaveType));
    eventView->addValue ("ctrl", getControllerName ((int) e.controller));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2AssignablePerNoteControllerEvent (const UmpEvent& event)
{
    Midi2AssignablePerNoteControllerEvent e (event);
    eventView->setEvent ("MIDI 2 Assignable Per Note Controller");
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("note", getNoteName ((int) e.note, pc.eventViewContext.octaveType));
    eventView->addValue ("ctrl", getControllerName ((int) e.controller));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2PerNoteEvent (const UmpEvent& event)
{
    Midi2PerNoteEvent e (event);
    eventView->setEvent ("MIDI 2 Per Note");
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("note", getNoteName ((int) e.note, pc.eventViewContext.octaveType));
    eventView->addValue ("ctrl", getControllerName ((int) e.controller));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2PerNotePitchBendEvent (const UmpEvent& event)
{
    Midi2PerNotePitchBendEvent e (event);
    eventView->setEvent ("MIDI 2 Per Note Pitch Bend");
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("note", getNoteName ((int) e.note, pc.eventViewContext.octaveType));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision, -1.f, 1.f));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2ControlChangeEvent (const UmpEvent& event)
{
    Midi2ControlChangeEvent e (event);
    eventView->setEvent ("MIDI 2 Control Change");
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("ctrl", getControllerName ((int) e.controller));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2ProgramChangeEvent (const UmpEvent& event)
{
    Midi2ProgramChangeEvent e (event);
    eventView->setEvent ("MIDI 2 Program Change");
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("prog", juce::String ((int) e.program));
    if ((bool) e.bankValid)
        eventView->addValue ("bank", juce::String ((int) e.bank));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2PerNoteManagementEvent (const UmpEvent& event)
{
    Midi2PerNoteManagementEvent e (event);
    eventView->setEvent ("MIDI 2 Per Note Management");
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("note", getNoteName ((int) e.note, pc.eventViewContext.octaveType));
    eventView->addValue ("detach", (bool) e.detach ? "Y" : "N");
    eventView->addValue ("reset", (bool) e.reset ? "Y" : "N");
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2PolyPressureEvent (const UmpEvent& event)
{
    Midi2PolyPressureEvent e (event);
    eventView->setEvent ("MIDI 2 Poly Pressure");
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("note", getNoteName ((int) e.note, pc.eventViewContext.octaveType));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2ChannelPressureEvent (const UmpEvent& event)
{
    Midi2ChannelPressureEvent e (event);
    eventView->setEvent ("MIDI 2 Channel Pressure");
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2PitchBendEvent (const UmpEvent& event)
{
    Midi2PitchBendEvent e (event);
    eventView->setEvent ("MIDI 2 Pitch Bend");
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision, -1.f, 1.f));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2RegisteredControllerEvent (const UmpEvent& event)
{
    Midi2RegisteredControllerEvent e (event);
    eventView->setEvent ("MIDI 2 Registered Controller");
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("bank", juce::String ((int) e.bank));
    eventView->addValue ("ctrl", juce::String ((int) e.controller));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2AssignableControllerEvent (const UmpEvent& event)
{
    Midi2AssignableControllerEvent e (event);
    eventView->setEvent ("MIDI 2 Assignable Controller");
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("bank", juce::String ((int) e.bank));
    eventView->addValue ("ctrl", juce::String ((int) e.controller));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2ControllerEvent (const UmpEvent& event)
{
    Midi2ControllerEvent e (event);
    eventView->setEvent ("MIDI 2 Control Change");
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("bank", juce::String ((int) e.bank));
    eventView->addValue ("ctrl", juce::String ((int) e.controller));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2RelativeRegisteredControllerEvent (const UmpEvent& event)
{
    Midi2RelativeRegisteredControllerEvent e (event);
    eventView->setEvent ("MIDI 2 Relative Registered Controller");
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("bank", juce::String ((int) e.bank));
    eventView->addValue ("ctrl", juce::String ((int) e.controller));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2RelativeAssignableControllerEvent (const UmpEvent& event)
{
    Midi2RelativeAssignableControllerEvent e (event);
    eventView->setEvent ("MIDI 2 Relative Assignable Controller");
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("bank", juce::String ((int) e.bank));
    eventView->addValue ("ctrl", juce::String ((int) e.controller));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2RelativeControllerEvent (const UmpEvent& event)
{
    Midi2RelativeControllerEvent e (event);
    eventView->setEvent ("MIDI 2 Relative Control Change");
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("bank", juce::String ((int) e.bank));
    eventView->addValue ("ctrl", juce::String ((int) e.controller));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2ChannelVoiceEvent (const UmpEvent& event)
{
    Midi2ChannelVoiceEvent e (event);
    eventView->setEvent ("MIDI 2 Channel Voice");
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onUmpEvent (const UmpEvent& event)
{
    eventView->setEvent ("MIDI 2 UMP");
    eventView->addValue ("d0", juce::String::formatted ("%08X", event.getattr<uint32_t> (UmpWords::data0Id, 0)));
    eventView->addValue ("d1", juce::String::formatted ("%08X", event.getattr<uint32_t> (UmpWords::data1Id, 0)));
    return UmpHandler::Result::ok;
}
