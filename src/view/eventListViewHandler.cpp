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

#include "model/ump/channelVoice1.h"
#include "model/ump/channelVoice2.h"
#include "model/ump/systemCommon.h"
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
        case MessageTypes::systemCommon:
            labelColor = pal.systemCommonLabel.get ();
            valueColor = pal.systemCommonValue.get ();
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
            // add a newline iff we've already displayed parsed data (e.g., do NOT
            // add a line break for a NOOP message!)
            if (pc.eventViewContext.umpShowParsedData && eventView->getNumDataValues () > 0)
                eventView->addLine ();

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

UmpHandler::Result EventListViewHandler::addSystemCommonNoDataValues (const SystemCommonEvent& e)
{
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onSystemCommonEvent (const UmpEvent& event)
{
    SystemCommonEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("status", formatValue ((uint32_t) e.status, 8, ValueFormatType::Hex));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidiTimeCodeEvent (const UmpEvent& event)
{
    MidiTimeCodeEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("type", getMtcMessageTypeName (e.mtcType));
    eventView->addValue ("data", juce::String ((int) e.mtcData));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onSongPositionPointerEvent (const UmpEvent& event)
{
    SongPositionPointerEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("pos", juce::String ((int) e.value));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onSongSelectEvent (const UmpEvent& event)
{
    SongSelectEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("song", juce::String ((int) e.song));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onTuneRequestEvent (const UmpEvent& event)
{
    TuneRequestEvent e (event);
    return addSystemCommonNoDataValues (e);
}

UmpHandler::Result EventListViewHandler::onTimingClockEvent (const UmpEvent& event)
{
    TimingClockEvent e (event);
    return addSystemCommonNoDataValues (e);
}

UmpHandler::Result EventListViewHandler::onStartEvent (const UmpEvent& event)
{
    StartEvent e (event);
    return addSystemCommonNoDataValues (e);
}

UmpHandler::Result EventListViewHandler::onContinueEvent (const UmpEvent& event)
{
    ContinueEvent e (event);
    return addSystemCommonNoDataValues (e);
}

UmpHandler::Result EventListViewHandler::onStopEvent (const UmpEvent& event)
{
    StopEvent e (event);
    return addSystemCommonNoDataValues (e);
}

UmpHandler::Result EventListViewHandler::onActiveSensingEvent (const UmpEvent& event)
{
    ActiveSensingEvent e (event);
    return addSystemCommonNoDataValues (e);
}

UmpHandler::Result EventListViewHandler::onSystemResetEvent (const UmpEvent& event)
{
    SystemResetEvent e (event);
    return addSystemCommonNoDataValues (e);
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
    return addUtilityTicksValues (e);
}

UmpHandler::Result EventListViewHandler::onJrTimestampEvent (const UmpEvent& event)
{
    JrTimestampEvent e (event);
    return addUtilityTicksValues (e);
}

UmpHandler::Result EventListViewHandler::onDeltaTicksPerQuarterEvent (const UmpEvent& event)
{
    DeltaTicksPerQuarterEvent e (event);
    return addUtilityTicksValues (e);
}

UmpHandler::Result EventListViewHandler::onDeltaTicksSinceLastEvent (const UmpEvent& event)
{
    DeltaTicksSinceLastEvent e (event);
    return addUtilityTicksValues (e);
}

UmpHandler::Result EventListViewHandler::onMidi1NoteOffEvent (const UmpEvent& event)
{
    Midi1NoteOffEvent e (event);
    return addMidi1NoteValues (e);
}

UmpHandler::Result EventListViewHandler::onMidi1NoteOnEvent (const UmpEvent& event)
{
    Midi1NoteOnEvent e (event);
    return addMidi1NoteValues (e);
}

UmpHandler::Result EventListViewHandler::onMidi1NoteEvent (const UmpEvent& event)
{
    Midi1NoteEvent e (event);
    return addMidi1NoteValues (e);
}

UmpHandler::Result EventListViewHandler::onMidi1PolyPressureEvent (const UmpEvent& event)
{
    Midi1PolyPressureEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("note", getNoteName ((int) e.note, pc.eventViewContext.octaveType));
    eventView->addValue ("val", formatValue ((uint32_t) e.pressure, 7, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi1ControlChangeEvent (const UmpEvent& event)
{
    Midi1ControlChangeEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("ctrl", getControllerName ((int) e.controller));
    eventView->addValue (
        "val", formatValue ((uint32_t) e.value, 7, pc.eventViewContext.valueFormatType, pc.eventViewContext.precision));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi1ProgramChangeEvent (const UmpEvent& event)
{
    Midi1ProgramChangeEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("prog", juce::String ((int) e.program));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi1ChannelPressureEvent (const UmpEvent& event)
{
    Midi1ChannelPressureEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("val", formatValue ((uint32_t) e.pressure, 7, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi1PitchBendEvent (const UmpEvent& event)
{
    Midi1PitchBendEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 14, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision, -1.f, 1.f));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::onMidi1ChannelVoiceEvent (const UmpEvent& event)
{
    Midi1ChannelVoiceEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::addMidi2NoteValues (const Midi2NoteEvent& e)
{
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("note", getNoteName ((int) e.note, pc.eventViewContext.octaveType));
    eventView->addValue ("vel", formatValue ((uint32_t) e.velocity, 16, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    if (e.attributeType.get () != 0)
    {
        eventView->addValue ("attrType", getNoteAttributeName ((int) e.attributeType));
        eventView->addValue ("attrVal",
                             formatValue ((uint32_t) e.attributeValue, 16, pc.eventViewContext.valueFormatType,
                                          pc.eventViewContext.precision));
    }
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::addUtilityTicksValues (const UtilityEvent16T& e)
{
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    eventView->addValue ("ticks", juce::String ((int) e.ticks));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::addUtilityTicksValues (const DeltaTicksSinceLastEvent& e)
{
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    eventView->addValue ("ticks", juce::String ((int) e.ticks));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::addMidi1NoteValues (const Midi1NoteEvent& e)
{
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return UmpHandler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("note", getNoteName ((int) e.note, pc.eventViewContext.octaveType));
    eventView->addValue ("vel", formatValue ((uint32_t) e.velocity, 7, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return UmpHandler::Result::ok;
}

UmpHandler::Result EventListViewHandler::addMidi2PerNoteValues (const Midi2PerNoteEvent& e)
{
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

UmpHandler::Result EventListViewHandler::addMidi2ControllerValues (const Midi2ControllerEvent& e)
{
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

UmpHandler::Result EventListViewHandler::addMidi2RelativeControllerValues (const Midi2RelativeControllerEvent& e)
{
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

UmpHandler::Result EventListViewHandler::onMidi2NoteOffEvent (const UmpEvent& event)
{
    Midi2NoteOffEvent e (event);
    return addMidi2NoteValues (e);
}

UmpHandler::Result EventListViewHandler::onMidi2NoteOnEvent (const UmpEvent& event)
{
    Midi2NoteOnEvent e (event);
    return addMidi2NoteValues (e);
}

UmpHandler::Result EventListViewHandler::onMidi2NoteEvent (const UmpEvent& event)
{
    Midi2NoteEvent e (event);
    return addMidi2NoteValues (e);
}

UmpHandler::Result EventListViewHandler::onMidi2RegisteredPerNoteControllerEvent (const UmpEvent& event)
{
    Midi2RegisteredPerNoteControllerEvent e (event);
    return addMidi2PerNoteValues (e);
}

UmpHandler::Result EventListViewHandler::onMidi2AssignablePerNoteControllerEvent (const UmpEvent& event)
{
    Midi2AssignablePerNoteControllerEvent e (event);
    return addMidi2PerNoteValues (e);
}

UmpHandler::Result EventListViewHandler::onMidi2PerNoteEvent (const UmpEvent& event)
{
    Midi2PerNoteEvent e (event);
    return addMidi2PerNoteValues (e);
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
    return addMidi2ControllerValues (e);
}

UmpHandler::Result EventListViewHandler::onMidi2AssignableControllerEvent (const UmpEvent& event)
{
    Midi2AssignableControllerEvent e (event);
    return addMidi2ControllerValues (e);
}

UmpHandler::Result EventListViewHandler::onMidi2ControllerEvent (const UmpEvent& event)
{
    Midi2ControllerEvent e (event);
    return addMidi2ControllerValues (e);
}

UmpHandler::Result EventListViewHandler::onMidi2RelativeRegisteredControllerEvent (const UmpEvent& event)
{
    Midi2RelativeRegisteredControllerEvent e (event);
    return addMidi2RelativeControllerValues (e);
}

UmpHandler::Result EventListViewHandler::onMidi2RelativeAssignableControllerEvent (const UmpEvent& event)
{
    Midi2RelativeAssignableControllerEvent e (event);
    return addMidi2RelativeControllerValues (e);
}

UmpHandler::Result EventListViewHandler::onMidi2RelativeControllerEvent (const UmpEvent& event)
{
    Midi2RelativeControllerEvent e (event);
    return addMidi2RelativeControllerValues (e);
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
