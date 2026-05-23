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
#include "model/ump/flexData.h"
#include "model/ump/stream.h"
#include "model/ump/sysex7.h"
#include "model/ump/sysex8.h"
#include "model/ump/systemCommon.h"
#include "model/ump/utility.h"

#include "palette.h"
#include "view/dispatchContext.h"

EventListViewHandler::EventListViewHandler (AppContext& theAppContext)
: appContext (theAppContext)
, pc { theAppContext }
{
}

EventListViewHandler::~EventListViewHandler ()
{
    eventView = nullptr;
}

Handler::Result EventListViewHandler::handle (const UmpEvent& event, void* ctx)
{
    if (ctx)
    {
        auto* dispCtx = static_cast<DispatchContext*> (ctx);
        eventView     = dispCtx->view;
        currentIndex  = dispCtx->index;
        currentWidth  = dispCtx->width;
    }

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

    return UmpHandler::handle (event);
}

Handler::Result EventListViewHandler::preDispatch (const UmpEvent& event)
{
    eventView->setTime (formatTime ((double) event.timestamp));

    eventView->setEndpoint (formatEndpoint (event));

    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::postDispatch (const UmpEvent& event, Handler::Result pendingResult)
{
    if (pendingResult == Handler::Result::ok)
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

            eventView->addValue ("ump", rawDataStr.trimEnd ());
        }
        eventView->sizeToWidth (currentWidth);
    }

    eventView    = nullptr;
    currentIndex = -1;
    currentWidth = 0;
    return pendingResult;
}

Handler::Result EventListViewHandler::addSystemCommonNoDataValues (const SystemCommonEvent& e)
{
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onSystemCommonEvent (const UmpEvent& event)
{
    SystemCommonEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("status", formatValue ((uint32_t) e.status, 8, ValueFormatType::Hex));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onMidiTimeCodeEvent (const UmpEvent& event)
{
    MidiTimeCodeEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("type", getMtcMessageTypeName (e.mtcType));
    eventView->addValue ("data", juce::String ((int) e.mtcData));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onSongPositionPointerEvent (const UmpEvent& event)
{
    SongPositionPointerEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("pos", juce::String ((int) e.value));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onSongSelectEvent (const UmpEvent& event)
{
    SongSelectEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("song", juce::String ((int) e.song));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onTuneRequestEvent (const UmpEvent& event)
{
    TuneRequestEvent e (event);
    return addSystemCommonNoDataValues (e);
}

Handler::Result EventListViewHandler::onTimingClockEvent (const UmpEvent& event)
{
    TimingClockEvent e (event);
    return addSystemCommonNoDataValues (e);
}

Handler::Result EventListViewHandler::onStartEvent (const UmpEvent& event)
{
    StartEvent e (event);
    return addSystemCommonNoDataValues (e);
}

Handler::Result EventListViewHandler::onContinueEvent (const UmpEvent& event)
{
    ContinueEvent e (event);
    return addSystemCommonNoDataValues (e);
}

Handler::Result EventListViewHandler::onStopEvent (const UmpEvent& event)
{
    StopEvent e (event);
    return addSystemCommonNoDataValues (e);
}

Handler::Result EventListViewHandler::onActiveSensingEvent (const UmpEvent& event)
{
    ActiveSensingEvent e (event);
    return addSystemCommonNoDataValues (e);
}

Handler::Result EventListViewHandler::onSystemResetEvent (const UmpEvent& event)
{
    SystemResetEvent e (event);
    return addSystemCommonNoDataValues (e);
}

Handler::Result EventListViewHandler::onNoOpEvent (const UmpEvent& event)
{
    NoOpEvent e (event);
    eventView->setEvent (event.eventName);
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onJrClockEvent (const UmpEvent& event)
{
    JrClockEvent e (event);
    return addUtilityTicksValues (e);
}

Handler::Result EventListViewHandler::onJrTimestampEvent (const UmpEvent& event)
{
    JrTimestampEvent e (event);
    return addUtilityTicksValues (e);
}

Handler::Result EventListViewHandler::onDeltaTicksPerQuarterEvent (const UmpEvent& event)
{
    DeltaTicksPerQuarterEvent e (event);
    return addUtilityTicksValues (e);
}

Handler::Result EventListViewHandler::onDeltaTicksSinceLastEvent (const UmpEvent& event)
{
    DeltaTicksSinceLastEvent e (event);
    return addUtilityTicksValues (e);
}

Handler::Result EventListViewHandler::onMidi1NoteOffEvent (const UmpEvent& event)
{
    Midi1NoteOffEvent e (event);
    return addMidi1NoteValues (e);
}

Handler::Result EventListViewHandler::onMidi1NoteOnEvent (const UmpEvent& event)
{
    Midi1NoteOnEvent e (event);
    return addMidi1NoteValues (e);
}

Handler::Result EventListViewHandler::onMidi1NoteEvent (const UmpEvent& event)
{
    Midi1NoteEvent e (event);
    return addMidi1NoteValues (e);
}

Handler::Result EventListViewHandler::onMidi1PolyPressureEvent (const UmpEvent& event)
{
    Midi1PolyPressureEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("note", getNoteName ((int) e.note, pc.eventViewContext.octaveType));
    eventView->addValue ("val", formatValue ((uint32_t) e.pressure, 7, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onMidi1ControlChangeEvent (const UmpEvent& event)
{
    Midi1ControlChangeEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("ctrl", getControllerName ((int) e.controller));
    eventView->addValue (
        "val", formatValue ((uint32_t) e.value, 7, pc.eventViewContext.valueFormatType, pc.eventViewContext.precision));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onMidi1ProgramChangeEvent (const UmpEvent& event)
{
    Midi1ProgramChangeEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("prog", juce::String ((int) e.program));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onMidi1ChannelPressureEvent (const UmpEvent& event)
{
    Midi1ChannelPressureEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("val", formatValue ((uint32_t) e.pressure, 7, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onMidi1PitchBendEvent (const UmpEvent& event)
{
    Midi1PitchBendEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 14, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision, -1.f, 1.f));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onMidi1ChannelVoiceEvent (const UmpEvent& event)
{
    Midi1ChannelVoiceEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::addMidi2NoteValues (const Midi2NoteEvent& e)
{
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
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
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::addUtilityTicksValues (const UtilityEvent16T& e)
{
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("ticks", juce::String ((int) e.ticks));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::addUtilityTicksValues (const DeltaTicksSinceLastEvent& e)
{
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("ticks", juce::String ((int) e.ticks));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::addMidi1NoteValues (const Midi1NoteEvent& e)
{
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("note", getNoteName ((int) e.note, pc.eventViewContext.octaveType));
    eventView->addValue ("vel", formatValue ((uint32_t) e.velocity, 7, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::addMidi2PerNoteValues (const Midi2PerNoteEvent& e)
{
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("note", getNoteName ((int) e.note, pc.eventViewContext.octaveType));
    eventView->addValue ("ctrl", getControllerName ((int) e.controller));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::addMidi2ControllerValues (const Midi2ControllerEvent& e)
{
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("bank", juce::String ((int) e.bank));
    eventView->addValue ("ctrl", juce::String ((int) e.controller));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::addMidi2RelativeControllerValues (const Midi2RelativeControllerEvent& e)
{
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("bank", juce::String ((int) e.bank));
    eventView->addValue ("ctrl", juce::String ((int) e.controller));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onMidi2NoteOffEvent (const UmpEvent& event)
{
    Midi2NoteOffEvent e (event);
    return addMidi2NoteValues (e);
}

Handler::Result EventListViewHandler::onMidi2NoteOnEvent (const UmpEvent& event)
{
    Midi2NoteOnEvent e (event);
    return addMidi2NoteValues (e);
}

Handler::Result EventListViewHandler::onMidi2NoteEvent (const UmpEvent& event)
{
    Midi2NoteEvent e (event);
    return addMidi2NoteValues (e);
}

Handler::Result EventListViewHandler::onMidi2RegisteredPerNoteControllerEvent (const UmpEvent& event)
{
    Midi2RegisteredPerNoteControllerEvent e (event);
    return addMidi2PerNoteValues (e);
}

Handler::Result EventListViewHandler::onMidi2AssignablePerNoteControllerEvent (const UmpEvent& event)
{
    Midi2AssignablePerNoteControllerEvent e (event);
    return addMidi2PerNoteValues (e);
}

Handler::Result EventListViewHandler::onMidi2PerNoteEvent (const UmpEvent& event)
{
    Midi2PerNoteEvent e (event);
    return addMidi2PerNoteValues (e);
}

Handler::Result EventListViewHandler::onMidi2PerNotePitchBendEvent (const UmpEvent& event)
{
    Midi2PerNotePitchBendEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("note", getNoteName ((int) e.note, pc.eventViewContext.octaveType));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision, -1.f, 1.f));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onMidi2ControlChangeEvent (const UmpEvent& event)
{
    Midi2ControlChangeEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("ctrl", getControllerName ((int) e.controller));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onMidi2ProgramChangeEvent (const UmpEvent& event)
{
    Midi2ProgramChangeEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("prog", juce::String ((int) e.program));
    if ((bool) e.bankValid)
        eventView->addValue ("bank", juce::String ((int) e.bank));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onMidi2PerNoteManagementEvent (const UmpEvent& event)
{
    Midi2PerNoteManagementEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("note", getNoteName ((int) e.note, pc.eventViewContext.octaveType));
    eventView->addValue ("detach", (bool) e.detach ? "Y" : "N");
    eventView->addValue ("reset", (bool) e.reset ? "Y" : "N");
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onMidi2PolyPressureEvent (const UmpEvent& event)
{
    Midi2PolyPressureEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("note", getNoteName ((int) e.note, pc.eventViewContext.octaveType));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onMidi2ChannelPressureEvent (const UmpEvent& event)
{
    Midi2ChannelPressureEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onMidi2PitchBendEvent (const UmpEvent& event)
{
    Midi2PitchBendEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    eventView->addValue ("val", formatValue ((uint32_t) e.value, 32, pc.eventViewContext.valueFormatType,
                                             pc.eventViewContext.precision, -1.f, 1.f));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onMidi2RegisteredControllerEvent (const UmpEvent& event)
{
    Midi2RegisteredControllerEvent e (event);
    return addMidi2ControllerValues (e);
}

Handler::Result EventListViewHandler::onMidi2AssignableControllerEvent (const UmpEvent& event)
{
    Midi2AssignableControllerEvent e (event);
    return addMidi2ControllerValues (e);
}

Handler::Result EventListViewHandler::onMidi2ControllerEvent (const UmpEvent& event)
{
    Midi2ControllerEvent e (event);
    return addMidi2ControllerValues (e);
}

Handler::Result EventListViewHandler::onMidi2RelativeRegisteredControllerEvent (const UmpEvent& event)
{
    Midi2RelativeRegisteredControllerEvent e (event);
    return addMidi2RelativeControllerValues (e);
}

Handler::Result EventListViewHandler::onMidi2RelativeAssignableControllerEvent (const UmpEvent& event)
{
    Midi2RelativeAssignableControllerEvent e (event);
    return addMidi2RelativeControllerValues (e);
}

Handler::Result EventListViewHandler::onMidi2RelativeControllerEvent (const UmpEvent& event)
{
    Midi2RelativeControllerEvent e (event);
    return addMidi2RelativeControllerValues (e);
}

Handler::Result EventListViewHandler::onMidi2ChannelVoiceEvent (const UmpEvent& event)
{
    Midi2ChannelVoiceEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("ch", juce::String ((int) e.userChannel));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::addSysex7DataValues (const Sysex7Event& e)
{
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("status", getSysexStatusName (e.status));
    const bool isInteger = (pc.eventViewContext.valueFormatType == ValueFormatType::Decimal);
    juce::String bytesStr;
    for (int i = 0; i < (int) e.numBytes; ++i)
    {
        if (i > 0)
            bytesStr += " ";
        const int b = e[i];
        bytesStr += isInteger ? juce::String (b) : juce::String::toHexString (b).paddedLeft ('0', 2).toUpperCase ();
    }
    if (bytesStr.isNotEmpty ())
        eventView->addValue ("data", bytesStr);
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onSysex7CompleteEvent (const UmpEvent& event)
{
    Sysex7Event e (event);
    return addSysex7DataValues (e);
}

Handler::Result EventListViewHandler::onSysex7StartEvent (const UmpEvent& event)
{
    Sysex7Event e (event);
    return addSysex7DataValues (e);
}

Handler::Result EventListViewHandler::onSysex7ContinueEvent (const UmpEvent& event)
{
    Sysex7Event e (event);
    return addSysex7DataValues (e);
}

Handler::Result EventListViewHandler::onSysex7EndEvent (const UmpEvent& event)
{
    Sysex7Event e (event);
    return addSysex7DataValues (e);
}

Handler::Result EventListViewHandler::onSysex7Event (const UmpEvent& event)
{
    Sysex7Event e (event);
    return addSysex7DataValues (e);
}

Handler::Result EventListViewHandler::addSysex8DataValues (const Sysex8Event& e)
{
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;

    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("status", getSysexStatusName (e.status));
    eventView->addValue ("sid", juce::String::toHexString ((int) e.streamId).paddedLeft ('0', 2).toUpperCase ());

    // numBytes includes the stream ID; actual data byte count is numBytes - 1,
    // clamped to 0–13. The special unknown-quality value (0xF) produces 14 here
    // and is intentionally skipped.
    const int numDataBytes = std::min ((int) e.numBytes - 1, 13);
    if (numDataBytes > 0)
    {
        const bool isInteger = (pc.eventViewContext.valueFormatType == ValueFormatType::Decimal);
        juce::String bytesStr;
        for (int i = 0; i < numDataBytes; ++i)
        {
            if (i > 0)
                bytesStr += " ";
            const int b = e[i];
            bytesStr += isInteger ? juce::String (b) : juce::String::toHexString (b).paddedLeft ('0', 2).toUpperCase ();
        }
        eventView->addValue ("data", bytesStr);
    }

    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onSysex8CompleteEvent (const UmpEvent& event)
{
    Sysex8Event e (event);
    return addSysex8DataValues (e);
}

Handler::Result EventListViewHandler::onSysex8StartEvent (const UmpEvent& event)
{
    Sysex8Event e (event);
    return addSysex8DataValues (e);
}

Handler::Result EventListViewHandler::onSysex8ContinueEvent (const UmpEvent& event)
{
    Sysex8Event e (event);
    return addSysex8DataValues (e);
}

Handler::Result EventListViewHandler::onSysex8EndEvent (const UmpEvent& event)
{
    Sysex8Event e (event);
    return addSysex8DataValues (e);
}

Handler::Result EventListViewHandler::onSysex8Event (const UmpEvent& event)
{
    Sysex8Event e (event);
    return addSysex8DataValues (e);
}

Handler::Result EventListViewHandler::onMixedDataSetHeaderEvent (const UmpEvent& event)
{
    MixedDataSetHeaderEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("mds", juce::String ((int) e.mdsId));
    eventView->addValue ("bytes", juce::String ((int) e.numValidBytes));
    eventView->addValue ("chunks", juce::String ((int) e.numChunks));
    eventView->addValue ("chunk", juce::String ((int) e.chunkNumber));
    eventView->addValue ("mfr", juce::String::toHexString ((int) e.manufacturerId).paddedLeft ('0', 4).toUpperCase ());
    eventView->addValue ("dev", juce::String::toHexString ((int) e.deviceId).paddedLeft ('0', 4).toUpperCase ());
    eventView->addValue ("sub1", juce::String::toHexString ((int) e.subId1).paddedLeft ('0', 4).toUpperCase ());
    eventView->addValue ("sub2", juce::String::toHexString ((int) e.subId2).paddedLeft ('0', 4).toUpperCase ());
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onMixedDataSetPayloadEvent (const UmpEvent& event)
{
    MixedDataSetPayloadEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("mds", juce::String ((int) e.mdsId));
    const bool isInteger = (pc.eventViewContext.valueFormatType == ValueFormatType::Decimal);
    juce::String bytesStr;
    for (int i = 0; i < 14; ++i)
    {
        if (i > 0)
            bytesStr += " ";
        const int b = e[i];
        bytesStr += isInteger ? juce::String (b) : juce::String::toHexString (b).paddedLeft ('0', 2).toUpperCase ();
    }
    eventView->addValue ("data", bytesStr);
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onSetTempoEvent (const UmpEvent& event)
{
    SetTempoEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    const uint32_t tenNs = e.tenNsPerQuarterNote;
    const double bpm     = tenNsToBpm (tenNs);
    eventView->addValue ("tempo", juce::String (tenNs) + " (" + juce::String (bpm, 3) + " bpm)");
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onSetTimeSignatureEvent (const UmpEvent& event)
{
    SetTimeSignatureEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("num", juce::String ((int) e.numerator));
    const int dp                = e.denominatorPower;
    const juce::String denomStr = (dp == 0) ? "non-standard" : juce::String (1 << dp);
    eventView->addValue ("denom", denomStr);
    eventView->addValue ("32nds", juce::String ((int) e.num32ndNotes));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onSetMetronomeEvent (const UmpEvent& event)
{
    SetMetronomeEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));
    eventView->addValue ("clocks", juce::String ((int) e.numClocksPerPrimaryClick));

    juce::String accentStr = juce::String ((int) e.barAccentPart1);
    if (const int p2 = e.barAccentPart2; p2 != 0)
    {
        accentStr += "+" + juce::String (p2);
        if (const int p3 = e.barAccentPart3; p3 != 0)
            accentStr += "+" + juce::String (p3);
    }
    eventView->addValue ("acc", accentStr);

    if (const int s1 = e.numSubdivisionClicks1; s1 != 0)
        eventView->addValue ("sub1", juce::String (s1));
    if (const int s2 = e.numSubdivisionClicks2; s2 != 0)
        eventView->addValue ("sub2", juce::String (s2));

    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onSetKeySignatureEvent (const UmpEvent& event)
{
    SetKeySignatureEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));

    const int sf = e.sharpsFlats;
    juce::String sfStr;
    if (sf == -8)
        sfStr = "unknown";
    else if (sf == 0)
        sfStr = "0";
    else if (sf > 0)
        sfStr = "+" + juce::String (sf) + " (" + juce::String (sf) + " sharp" + (sf == 1 ? "" : "s") + ")";
    else
        sfStr = juce::String (sf) + " (" + juce::String (-sf) + " flat" + (sf == -1 ? "" : "s") + ")";
    eventView->addValue ("key", sfStr);

    static constexpr const char* noteNames[] = { "unknown", "A", "B", "C", "D", "E", "F", "G" };
    const int tn                             = static_cast<int> (e.tonicNote.get ());
    eventView->addValue ("tonic", (tn >= 0 && tn <= 7) ? noteNames[tn] : "reserved");

    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onMetadataTextEvent (const UmpEvent& event)
{
    return handleTextEvent (event);
}

Handler::Result EventListViewHandler::onPerformanceTextEvent (const UmpEvent& event)
{
    return handleTextEvent (event);
}

Handler::Result EventListViewHandler::handleTextEvent (const UmpEvent& event)
{
    FlexDataTextEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;

    eventView->addValue ("grp", juce::String ((int) e.userGroup));

    static constexpr const char* fmtNames[] = { "complete", "start", "continue", "end" };
    const int fi                            = static_cast<int> (e.format.get ());
    eventView->addValue ("fmt", (fi >= 0 && fi <= 3) ? fmtNames[fi] : "?");

    const bool isInteger = (pc.eventViewContext.valueFormatType == ValueFormatType::Decimal);
    juce::String bytesStr;
    for (int i = 0; i < FlexDataTextEvent::maxBytes; ++i)
    {
        const uint8_t b = e[i];
        if (b == 0)
            break;
        if (i > 0)
            bytesStr += " ";
        if (b >= 0x20 && b <= 0x7E)
            bytesStr += juce::String ("'") + juce::String::charToString ((juce::juce_wchar) b) + "'";
        else
            bytesStr += isInteger ? juce::String ((int) b)
                                  : juce::String::toHexString ((int) b).paddedLeft ('0', 2).toUpperCase ();
    }
    if (bytesStr.isNotEmpty ())
        eventView->addValue ("data", bytesStr);

    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onSetChordEvent (const UmpEvent& event)
{
    SetChordEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("grp", juce::String ((int) e.userGroup));

    static constexpr const char* noteNames[] = { "unknown", "A", "B", "C", "D", "E", "F", "G" };

    auto accStr = [] (int sf) -> const char*
    {
        switch (sf)
        {
            case -2:
                return "bb";
            case -1:
                return "b";
            case 1:
                return "#";
            case 2:
                return "x";
            default:
                return "";
        }
    };

    auto chordName = [] (ChordType ct) -> const char*
    {
        switch (ct)
        {
            case ChordType::noChord:
                return "No Chord";
            case ChordType::major:
                return "Major";
            case ChordType::major6th:
                return "Major 6th";
            case ChordType::major7th:
                return "Major 7th";
            case ChordType::major9th:
                return "Major 9th";
            case ChordType::major11th:
                return "Major 11th";
            case ChordType::major13th:
                return "Major 13th";
            case ChordType::minor:
                return "Minor";
            case ChordType::minor6th:
                return "Minor 6th";
            case ChordType::minor7th:
                return "Minor 7th";
            case ChordType::minor9th:
                return "Minor 9th";
            case ChordType::minor11th:
                return "Minor 11th";
            case ChordType::minor13th:
                return "Minor 13th";
            case ChordType::dominant:
                return "Dominant";
            case ChordType::dominant9th:
                return "Dominant 9th";
            case ChordType::dominant11th:
                return "Dominant 11th";
            case ChordType::dominant13th:
                return "Dominant 13th";
            case ChordType::augmented:
                return "Augmented";
            case ChordType::augmented7th:
                return "Augmented 7th";
            case ChordType::diminished:
                return "Diminished";
            case ChordType::diminished7th:
                return "Diminished 7th";
            case ChordType::halfDiminished:
                return "Half Dim.";
            case ChordType::majorMinor:
                return "Major-Minor";
            case ChordType::pedal:
                return "Pedal";
            case ChordType::power:
                return "Power";
            case ChordType::suspended2nd:
                return "Sus2";
            case ChordType::suspended4th:
                return "Sus4";
            case ChordType::sevenSuspended4th:
                return "7Sus4";
            default:
                return "reserved";
        }
    };

    auto alterStr = [] (AlterationType t) -> const char*
    {
        switch (t)
        {
            case AlterationType::add:
                return "add";
            case AlterationType::subtract:
                return "sub";
            case AlterationType::raise:
                return "raise";
            case AlterationType::lower:
                return "lower";
            default:
                return "";
        }
    };

    const int tn                = static_cast<int> (e.chordTonic.get ());
    const juce::String tonicStr = juce::String (noteNames[tn >= 0 && tn <= 7 ? tn : 0]) + accStr (e.chordSharpsFlats);
    eventView->addValue ("chord", tonicStr + " " + chordName (e.chordType.get ()));

    auto showAlter = [&] (AlterationType t, int deg, const juce::String& label)
    {
        if (t != AlterationType::none)
            eventView->addValue (label, juce::String (alterStr (t)) + " " + juce::String (deg));
    };
    showAlter (e.alter1Type.get (), e.alter1Degree.get (), "alt1");
    showAlter (e.alter2Type.get (), e.alter2Degree.get (), "alt2");
    showAlter (e.alter3Type.get (), e.alter3Degree.get (), "alt3");
    showAlter (e.alter4Type.get (), e.alter4Degree.get (), "alt4");

    if (e.bassSharpsFlats != -8)
    {
        const int bn         = static_cast<int> (e.bassNote.get ());
        juce::String bassStr = juce::String (noteNames[bn >= 0 && bn <= 7 ? bn : 0]) + accStr (e.bassSharpsFlats);
        if (e.bassChordType.get () != ChordType::noChord)
            bassStr += juce::String (" / ") + chordName (e.bassChordType.get ());
        eventView->addValue ("bass", bassStr);
    }

    return Handler::Result::ok;
}

// ---------------------------------------------------------------------------
// Stream event handlers

Handler::Result EventListViewHandler::onEndpointDiscoveryEvent (const UmpEvent& event)
{
    EndpointDiscoveryEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("ver", juce::String ((int) e.umpVersionMajor) + "." + juce::String ((int) e.umpVersionMinor));
    eventView->addValue ("e", (bool) e.requestEndpointInfo ? "Y" : "N");
    eventView->addValue ("d", (bool) e.requestDeviceIdentity ? "Y" : "N");
    eventView->addValue ("n", (bool) e.requestEndpointName ? "Y" : "N");
    eventView->addValue ("i", (bool) e.requestProductInstanceId ? "Y" : "N");
    eventView->addValue ("s", (bool) e.requestStreamConfig ? "Y" : "N");
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onEndpointInfoNotificationEvent (const UmpEvent& event)
{
    EndpointInfoNotificationEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("ver", juce::String ((int) e.umpVersionMajor) + "." + juce::String ((int) e.umpVersionMinor));
    eventView->addValue ("static", (bool) e.staticFunctionBlocks ? "Y" : "N");
    eventView->addValue ("fb", juce::String ((int) e.numFunctionBlocks));
    eventView->addValue ("m2", (bool) e.midi2Protocol ? "Y" : "N");
    eventView->addValue ("m1", (bool) e.midi1Protocol ? "Y" : "N");
    eventView->addValue ("rxJR", (bool) e.rxJrTimestamp ? "Y" : "N");
    eventView->addValue ("txJR", (bool) e.txJrTimestamp ? "Y" : "N");
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onDeviceIdentityNotificationEvent (const UmpEvent& event)
{
    DeviceIdentityNotificationEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    auto hex2                 = [] (int v) { return juce::String::toHexString (v).paddedLeft ('0', 2).toUpperCase (); };
    const juce::String mfrStr = hex2 (e.mfrId1) + " " + hex2 (e.mfrId2) + " " + hex2 (e.mfrId3);
    eventView->addValue ("mfr", mfrStr);
    eventView->addValue ("family", juce::String ((int) e.deviceFamily));
    eventView->addValue ("model", juce::String ((int) e.modelNumber));
    eventView->addValue ("rev",
                         hex2 (e.swRev1) + " " + hex2 (e.swRev2) + " " + hex2 (e.swRev3) + " " + hex2 (e.swRev4));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::addStreamTextValues (const StreamTextEvent& e)
{
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    if (e.status == StreamStatus::functionBlockNameNotification)
        eventView->addValue ("fb", juce::String ((int) e.functionBlockNumber));
    static constexpr const char* fmtNames[] = { "complete", "start", "continue", "end" };
    const int fi                            = static_cast<int> (e.format.get ());
    eventView->addValue ("fmt", (fi >= 0 && fi <= 3) ? fmtNames[fi] : "?");
    const bool isInteger = (pc.eventViewContext.valueFormatType == ValueFormatType::Decimal);
    juce::String bytesStr;
    for (int i = 0; i < StreamTextEvent::maxBytes; ++i)
    {
        const uint8_t b = e[i];
        if (b == 0)
            break;
        if (i > 0)
            bytesStr += " ";
        if (b >= 0x20 && b <= 0x7E)
            bytesStr += juce::String ("'") + juce::String::charToString ((juce::juce_wchar) b) + "'";
        else
            bytesStr += isInteger ? juce::String ((int) b)
                                  : juce::String::toHexString ((int) b).paddedLeft ('0', 2).toUpperCase ();
    }
    if (bytesStr.isNotEmpty ())
        eventView->addValue ("data", bytesStr);
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onEndpointNameNotificationEvent (const UmpEvent& event)
{
    StreamTextEvent e (event);
    return addStreamTextValues (e);
}

Handler::Result EventListViewHandler::onProductInstanceIdEvent (const UmpEvent& event)
{
    StreamTextEvent e (event);
    return addStreamTextValues (e);
}

Handler::Result EventListViewHandler::addStreamConfigValues (const UmpEvent& event, int theStatus)
{
    StreamConfigurationRequestEvent e (event);
    eventView->setEvent (theStatus == StreamStatus::streamConfigRequest
                             ? juce::String (e.eventName)
                             : juce::String ("Stream: Stream Configuration Notification"));
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    const int proto             = e.protocol;
    const juce::String protoStr = (proto == 1) ? "MIDI 1.0" : (proto == 2) ? "MIDI 2.0" : juce::String (proto);
    eventView->addValue ("proto", protoStr);
    eventView->addValue ("rxJR", (bool) e.rxJrTimestamp ? "Y" : "N");
    eventView->addValue ("txJR", (bool) e.txJrTimestamp ? "Y" : "N");
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onStreamConfigRequestEvent (const UmpEvent& event)
{
    return addStreamConfigValues (event, StreamStatus::streamConfigRequest);
}

Handler::Result EventListViewHandler::onStreamConfigNotificationEvent (const UmpEvent& event)
{
    return addStreamConfigValues (event, StreamStatus::streamConfigNotification);
}

Handler::Result EventListViewHandler::onFunctionBlockDiscoveryEvent (const UmpEvent& event)
{
    FunctionBlockDiscoveryEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    const int fb = e.functionBlockNumber;
    eventView->addValue ("fb", fb == 0xFF ? "all" : juce::String (fb));
    eventView->addValue ("i", (bool) e.requestInfo ? "Y" : "N");
    eventView->addValue ("n", (bool) e.requestName ? "Y" : "N");
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onFunctionBlockInfoNotificationEvent (const UmpEvent& event)
{
    FunctionBlockInfoNotificationEvent e (event);
    eventView->setEvent (e.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("active", (bool) e.active ? "Y" : "N");
    eventView->addValue ("fb", juce::String ((int) e.functionBlockNumber));
    static constexpr const char* dirNames[] = { "reserved", "input", "output", "bidir" };
    eventView->addValue ("dir", dirNames[(int) e.direction & 0x3]);
    eventView->addValue ("ui", juce::String ((int) e.uiHint));
    eventView->addValue ("m1", juce::String ((int) e.midi1));
    const int fg        = e.firstGroup;
    const int ng        = e.numGroups;
    juce::String grpStr = juce::String (fg + 1);
    if (ng > 1)
        grpStr += "-" + juce::String (fg + ng);
    eventView->addValue ("grp", grpStr);
    eventView->addValue ("m1ch", juce::String ((int) e.numMidi1Channels));
    eventView->addValue ("sx8", juce::String ((int) e.maxSysex8Streams));
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onFunctionBlockNameNotificationEvent (const UmpEvent& event)
{
    StreamTextEvent e (event);
    return addStreamTextValues (e);
}

Handler::Result EventListViewHandler::onStartOfClipEvent (const UmpEvent& event)
{
    StartOfClipEvent e (event);
    eventView->setEvent (e.eventName);
    return Handler::Result::ok;
}

Handler::Result EventListViewHandler::onEndOfClipEvent (const UmpEvent& event)
{
    EndOfClipEvent e (event);
    eventView->setEvent (e.eventName);
    return Handler::Result::ok;
}

// ---------------------------------------------------------------------------

Handler::Result EventListViewHandler::onUmpEvent (const UmpEvent& event)
{
    eventView->setEvent (event.eventName);
    if (!pc.eventViewContext.umpShowParsedData)
        return Handler::Result::ok;
    eventView->addValue ("d0", juce::String::formatted ("%08X", event.getattr<uint32_t> (UmpWords::data0Id, 0)));
    eventView->addValue ("d1", juce::String::formatted ("%08X", event.getattr<uint32_t> (UmpWords::data1Id, 0)));
    return Handler::Result::ok;
}
