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

#include "syntheticEndpointController.h"

#include "model/ump/channelVoice1.h"
#include "model/ump/channelVoice2.h"
#include "model/ump/systemCommon.h"
#include "model/ump/sysex7.h"
#include "model/ump/utility.h"

SyntheticEndpointController::SyntheticEndpointController (const MidiProperties& mp,
                                                           const juce::String& name)
: midiProperties { mp }
, midiEndpointProperties { juce::ump::EndpointId {} }
{
    midiEndpointProperties.name         = name;
    midiEndpointProperties.isInputAlive = true;
    midiEndpointProperties.isSynthetic  = true;
    midiProperties.endpoints.append (&midiEndpointProperties);

    midiEndpointProperties.playRequested.onPropertyChange (
        [this] (const juce::Identifier&)
        {
            if (midiEndpointProperties.playRequested.get ())
                startPlayback ();
        });

    buildDefaultEventList ();
}

void SyntheticEndpointController::startPlayback ()
{
    if (eventList.eventList.empty ())
        return;
    nextEventIndex = 0;
    nextFireTimeMs = juce::Time::currentTimeMillis () + eventList.eventList[0].deltaTimeMs;
    playing        = true;
    midiEndpointProperties.playRequested = false;
}

void SyntheticEndpointController::processUmpEvents ()
{
    if (!playing)
        return;

    const auto nowMs = juce::Time::currentTimeMillis ();
    while (playing && nowMs >= nextFireTimeMs)
    {
        if (startTime < 0)
            startTime = juce::Time::getMillisecondCounterHiRes () / 1000.0;
        const double elapsed = juce::Time::getMillisecondCounterHiRes () / 1000.0 - startTime;

        auto& entry = eventList.eventList[nextEventIndex];
        UmpEvent copy (entry.event.clone (false));
        copy.timestamp    = elapsed;
        copy.endpointName = midiEndpointProperties.name.get ();
        copy.isReceived   = true;
        midiEndpointProperties.received.addEvent (copy);

        ++nextEventIndex;
        if (nextEventIndex >= eventList.eventList.size ())
        {
            playing = false;
            break;
        }
        nextFireTimeMs += eventList.eventList[nextEventIndex].deltaTimeMs;
    }
}

void SyntheticEndpointController::buildDefaultEventList ()
{
    addUtilityEvents ();
    addSystemCommonEvents ();
    addSysex7Events ();
    addMidi1Events ();
    addMidi2Events ();
}

void SyntheticEndpointController::addUtilityEvents ()
{
    eventList.addEvent (100, NoOpEvent ());
    eventList.addEvent (100, JrClockEvent (480));
    eventList.addEvent (100, JrTimestampEvent (960));
    eventList.addEvent (100, DeltaTicksPerQuarterEvent (480));
    eventList.addEvent (100, DeltaTicksSinceLastEvent (240));
}

void SyntheticEndpointController::addSystemCommonEvents ()
{
    eventList.addEvent (100, MidiTimeCodeEvent       (1, MtcMessageType::secondsCountMsb, 9));
    eventList.addEvent (100, SongPositionPointerEvent (1, MidiWord { 1024 }));
    eventList.addEvent (100, SongSelectEvent          (1, MidiByte { 3 }));
    eventList.addEvent (100, TuneRequestEvent         (1));
    eventList.addEvent (100, TimingClockEvent         (1));
    eventList.addEvent (100, StartEvent               (1));
    eventList.addEvent (100, ContinueEvent            (1));
    eventList.addEvent (100, StopEvent                (1));
    eventList.addEvent (100, ActiveSensingEvent       (1));
    eventList.addEvent (100, SystemResetEvent         (1));
}

void SyntheticEndpointController::addSysex7Events ()
{
    // Complete messages: varying byte counts
    eventList.addEvent (100, Sysex7Event (1, SysexStatus::complete, 0));
    eventList.addEvent (100, Sysex7Event (1, SysexStatus::complete, 1, 0x41));
    eventList.addEvent (100, Sysex7Event (1, SysexStatus::complete, 3, 0x7E, 0x7F, 0x06));
    eventList.addEvent (100, Sysex7Event (1, SysexStatus::complete, 6, 0x7E, 0x7F, 0x09, 0x01, 0x00, 0x00));

    // Multi-packet: 16-byte payload split across start / continue / end
    eventList.addEvent (100, Sysex7Event (1, SysexStatus::start,     6, 0x41, 0x10, 0x42, 0x12, 0x40, 0x00));
    eventList.addEvent (100, Sysex7Event (1, SysexStatus::continue_, 6, 0x00, 0x7F, 0x00, 0x00, 0x41, 0x00));
    eventList.addEvent (100, Sysex7Event (1, SysexStatus::end,       4, 0x01, 0x02, 0x03, 0x00));

    // Factory: 20-byte payload auto-sliced into start / continue / continue / end
    const uint8_t payload[] = {
        0x7E, 0x7F, 0x06, 0x01,              // Universal Non-Realtime: Identity Request
        0x41, 0x10, 0x42, 0x12, 0x40, 0x00,  // Roland GS header
        0x00, 0x7F, 0x00, 0x00, 0x41, 0x00,  // GS parameter data
        0x01, 0x02, 0x03, 0x00               // trailing bytes
    };
    Sysex7EventFactory factory ([this] (const Sysex7Event& e)
                                { eventList.addEvent (100, e); });
    factory.createEvents (1, std::span (payload));
}

void SyntheticEndpointController::addMidi1Events ()
{
    eventList.addEvent (500, Midi1NoteOnEvent          (1, 1, 60, 100));
    eventList.addEvent (250, Midi1NoteOffEvent         (1, 1, 60, 0));
    eventList.addEvent (100, Midi1ControlChangeEvent   (1, 1, 7, 100));
    eventList.addEvent (100, Midi1PitchBendEvent       (1, 1, MidiWord { 12288 }));
    eventList.addEvent (100, Midi1PitchBendEvent       (1, 1, MidiWord { 8192 }));
    eventList.addEvent (150, Midi1PolyPressureEvent    (1, 1, 60, 64));
    eventList.addEvent (150, Midi1ChannelPressureEvent (1, 1, 80));
    eventList.addEvent (150, Midi1ProgramChangeEvent   (1, 1, 42));
}

void SyntheticEndpointController::addMidi2Events ()
{
    eventList.addEvent (1000, Midi2NoteOnEvent  (1, 1, 62, MidiUnipolarFloat (0.8f)));
    eventList.addEvent (250,  Midi2NoteOffEvent (1, 1, 62, MidiUnipolarFloat (0.25f)));
}
