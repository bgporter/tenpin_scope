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
#include "model/ump/utility.h"

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

    juce::Colour labelColor;
    juce::Colour valueColor;

    switch (event.messageType)
    {
        case MessageTypes::utility:
            labelColor = pal.utilityLabel.get ();
            valueColor = pal.utilityValue.get ();
            break;
        case MessageTypes::commonRealtime:
            labelColor = pal.commonRealtimeLabel.get ();
            valueColor = pal.commonRealtimeValue.get ();
            break;
        case MessageTypes::midi1ChannelVoice:
            labelColor = pal.midi1Label.get ();
            valueColor = pal.midi1Value.get ();
            break;
        case MessageTypes::sysex7:
            labelColor = pal.sysex7Label.get ();
            valueColor = pal.sysex7Value.get ();
            break;
        case MessageTypes::midi2ChannelVoice:
            labelColor = pal.midi2Label.get ();
            valueColor = pal.midi2Value.get ();
            break;
        case MessageTypes::sysex8:
            labelColor = pal.sysex8Label.get ();
            valueColor = pal.sysex8Value.get ();
            break;
        case MessageTypes::flexData:
            labelColor = pal.flexDataLabel.get ();
            valueColor = pal.flexDataValue.get ();
            break;
        case MessageTypes::stream:
            labelColor = pal.streamLabel.get ();
            valueColor = pal.streamValue.get ();
            break;
        default:
            labelColor = pal.undefinedLabel.get ();
            valueColor = pal.undefinedValue.get ();
            break;
    }

    eventView->setColors (pal.umpBackground.get (), labelColor, valueColor, pal.outline.get ());

    currentIndex = index;
    currentWidth = width;
    return UmpHandler::handle (event);
}

UmpHandler::Result EventListViewHandler::preDispatch (const UmpEvent& event)
{
    eventView->setTime (formatTime ((double) event.timestamp));

    const auto endpointStr = juce::String (event.endpointName) + " " + (event.isReceived ? "Rx" : "Tx");
    eventView->setEndpoint (endpointStr);

    return Result::ok;
}

UmpHandler::Result EventListViewHandler::postDispatch (const UmpEvent& event, UmpHandler::Result pendingResult)
{
    if (pendingResult == UmpHandler::Result::ok)
    {
        if (pc.eventViewContext.umpShowRawData || !pc.eventViewContext.umpShowParsedData)
        {
            if (pc.eventViewContext.umpShowParsedData)
                eventView->addLine ();
            // !!! actually get and format the correct hex data.

            juce::String rawDataStr;
            std::vector<juce::Identifier> dataIds { UmpWords::data0Id, UmpWords::data1Id, UmpWords::data2Id,
                                                    UmpWords::data3Id };
            for (const auto& dataId : dataIds)
            {
                if (!event.hasattr (dataId))
                    break;
                rawDataStr += formatValue (event.getattr<uint32_t> (dataId, 0), 32, ValueFormatType::Hex) + " ";
            }

            eventView->addValue ("raw", rawDataStr.trimEnd ());
        }
        eventView->sizeToWidth (currentWidth);
    }

    eventView    = nullptr;
    currentIndex = -1;
    currentWidth = 0;
    return pendingResult;
}

UmpHandler::Result EventListViewHandler::onNoOpEvent (const UmpEvent& event)
{
    NoOpEvent e (event);
    eventView->setEvent (event.eventName);
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onJrClockEvent (const UmpEvent& event)
{
    JrClockEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    // no group, no channel.
    eventView->addValue ("ticks", juce::String (e.ticks));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onJrTimestampEvent (const UmpEvent& event)
{
    JrTimestampEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    // no group, no channel.
    eventView->addValue ("ticks", juce::String (e.ticks));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onDeltaTicksPerQuarterEvent (const UmpEvent& event)
{
    DeltaTicksPerQuarterEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    // no group, no channel.
    eventView->addValue ("ticks", juce::String (e.ticks));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onDeltaTicksSinceLastEvent (const UmpEvent& event)
{
    DeltaTicksSinceLastEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    // no group, no channel.
    eventView->addValue ("ticks", juce::String (e.ticks));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2NoteOffEvent (const UmpEvent& event)
{
    Midi2NoteOffEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
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
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
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
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
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
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
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
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
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
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
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
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
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
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
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
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
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
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
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
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
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
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2PitchBendEvent (const UmpEvent& event)
{
    Midi2PitchBendEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision, -1.f, 1.f));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi2RegisteredControllerEvent (const UmpEvent& event)
{
    Midi2RegisteredControllerEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
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
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
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
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
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
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
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
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
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
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
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
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onUmpEvent (const UmpEvent& event)
{
    eventView->setEvent (event.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    eventView->addValue ("d0", juce::String::formatted ("%08X", event.getattr<uint32_t> (UmpWords::data0Id, 0)));
    eventView->addValue ("d1", juce::String::formatted ("%08X", event.getattr<uint32_t> (UmpWords::data1Id, 0)));
    return UmpHandler::Result::ok;
}
