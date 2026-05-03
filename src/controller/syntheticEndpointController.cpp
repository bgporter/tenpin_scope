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
#include "model/ump/flexData.h"
#include "model/ump/sysex8.h"
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
    addSysex8Events ();
    addMixedDataSetEvents ();
    addFlexDataEvents ();
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

void SyntheticEndpointController::addSysex8Events ()
{
    // Complete messages: empty (stream ID only), small, and full 13-byte
    eventList.addEvent (100, Sysex8Event (1, SysexStatus::complete, 0x01, std::span<const uint8_t> {}));
    const uint8_t threeBytes[] = { 0x41, 0x10, 0x42 };
    eventList.addEvent (100, Sysex8Event (1, SysexStatus::complete, 0x01, std::span (threeBytes)));
    const uint8_t fullPacket[] = { 0xFF, 0x80, 0xC0, 0xA0, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };
    eventList.addEvent (100, Sysex8Event (1, SysexStatus::complete, 0x01, std::span (fullPacket)));

    // Hand-built multi-packet sequence (stream ID 0x02)
    const uint8_t startBytes[]    = { 0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41, 0xFE, 0xDC, 0xBA, 0x98 };
    const uint8_t continueBytes[] = { 0x76, 0x54, 0x32, 0x10, 0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88, 0x77 };
    const uint8_t endBytes[]      = { 0x01, 0x02, 0x03 };
    eventList.addEvent (100, Sysex8Event (1, SysexStatus::start,     0x02, std::span (startBytes)));
    eventList.addEvent (100, Sysex8Event (1, SysexStatus::continue_, 0x02, std::span (continueBytes)));
    eventList.addEvent (100, Sysex8Event (1, SysexStatus::end,       0x02, std::span (endBytes)));

    // Special early-termination end packets
    eventList.addEvent (100, Sysex8Event (1, 0x03, Sysex8EndType::valid));
    eventList.addEvent (100, Sysex8Event (1, 0x03, Sysex8EndType::unknown));

    // Factory: 30-byte payload auto-sliced into start / continue / end (stream ID 0x04)
    const uint8_t payload[] = {
        0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41, 0x00, 0x01, 0x02, 0x03,
        0xFF, 0xEE, 0xDD, 0xCC, 0xBB, 0xAA, 0x99, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33,
        0x22, 0x11, 0x00, 0xAB
    };
    Sysex8EventFactory factory ([this] (const Sysex8Event& e)
                                { eventList.addEvent (100, e); });
    factory.createEvents (1, 0x04, std::span (payload));
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

void SyntheticEndpointController::addFlexDataEvents ()
{
    eventList.addEvent (100, SetTempoEvent (1, 120.0));
    eventList.addEvent (100, SetTempoEvent (1, 93.5));
    eventList.addEvent (100, SetTimeSignatureEvent (1, 4, 2, 8));    // 4/4, 8 x 1/32nd notes
    eventList.addEvent (100, SetTimeSignatureEvent (1, 6, 3, 8));    // 6/8, 8 x 1/32nd notes
    eventList.addEvent (100, SetMetronomeEvent (1, 24, 4, 0, 0, 2, 0));  // 4/4: 24 clocks/click, 4-beat accent, 2 subdivisions
    eventList.addEvent (100, SetMetronomeEvent (1, 12, 3, 2, 0, 3, 0));  // 5/4 (3+2), 12 clocks/click, 3 subdivisions
    eventList.addEvent (100, SetKeySignatureEvent (1,  0, TonicNote::c));  // C major
    eventList.addEvent (100, SetKeySignatureEvent (1,  1, TonicNote::g));  // G major (1 sharp)
    eventList.addEvent (100, SetKeySignatureEvent (1, -1, TonicNote::f));  // F major (1 flat)
    eventList.addEvent (100, SetKeySignatureEvent (1, -8, TonicNote::unknown));  // unknown key
}

void SyntheticEndpointController::addMidi2Events ()
{
    eventList.addEvent (1000, Midi2NoteOnEvent  (1, 1, 62, MidiUnipolarFloat (0.8f)));
    eventList.addEvent (250,  Midi2NoteOffEvent (1, 1, 62, MidiUnipolarFloat (0.25f)));
}

void SyntheticEndpointController::addMixedDataSetEvents ()
{
    // Hand-constructed: header + 2 payloads (20 bytes total, MDS ID 1)
    const uint8_t p1[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                            0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E };
    const uint8_t p2[] = { 0x0F, 0x10, 0x11, 0x12, 0x13 };
    // totalBytes = 14 + 5 = 19, 2 payload packets
    eventList.addEvent (100, MixedDataSetHeaderEvent  (1, 1, 19, 2, 1, 0x0041, 0x0010, 0x01, 0x01));
    eventList.addEvent (100, MixedDataSetPayloadEvent (1, 1, std::span (p1)));
    eventList.addEvent (100, MixedDataSetPayloadEvent (1, 1, std::span (p2)));

    // Factory: 30-byte payload → 1 header + 3 payloads (last has 2 bytes), MDS ID 2
    const uint8_t payload[] = {
        0x41, 0x10, 0x42, 0x12, 0x40, 0x00, 0x7F, 0x00, 0x41, 0xFF, 0xFE, 0xFD, 0xFC, 0xFB,
        0xFA, 0xF9, 0xF8, 0xF7, 0xF6, 0xF5, 0xF4, 0xF3, 0xF2, 0xF1, 0xF0, 0xEF, 0xEE, 0xED,
        0xEC, 0xEB
    };
    MixedDataSetFactory factory (
        [this] (const MixedDataSetHeaderEvent& h)  { eventList.addEvent (100, h); },
        [this] (const MixedDataSetPayloadEvent& p) { eventList.addEvent (100, p); });
    factory.createEvents (1, 2, 0x0041, 0x0010, 0x02, 0x03, std::span (payload));
}
