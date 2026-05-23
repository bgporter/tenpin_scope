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

#include "model/ci/profile.h"
#include "model/ci/propertyExchange.h"
#include "model/ci/discovery.h"
#include "model/ci/endpointInfo.h"
#include "model/ci/invalidateMuid.h"
#include "model/ci/ack.h"
#include "model/sysex/sysex7Message.h"
#include "model/ump/channelVoice1.h"
#include "model/ump/channelVoice2.h"
#include "model/ump/systemCommon.h"
#include "model/ump/sysex7.h"
#include "model/ump/flexData.h"
#include "model/ump/sysex8.h"
#include "model/ump/stream.h"
#include "model/ump/utility.h"

SyntheticEndpointController::SyntheticEndpointController (const MidiProperties& mp,
                                                           const juce::String& name,
                                                           AppContext& ac)
: midiProperties { mp }
, midiEndpointProperties { juce::ump::EndpointId {} }
, runtimeContext { ac }
{
    midiEndpointProperties.name         = name;
    midiEndpointProperties.isInputAlive = true;
    midiEndpointProperties.isSynthetic  = true;
    midiProperties.endpoints.append (&midiEndpointProperties);
    auto defer = [this] (Event& e) { midiEndpointProperties.deferMessage (e); };
    ciParser.emplace (defer);
    auto sysex7Defer = [this] (Event& e)
    {
        midiEndpointProperties.deferMessage (e);
        Sysex7Message msg { e };
        ciParser->parse (msg);
    };
    sysex7Builder.emplace (midiEndpointProperties.received, sysex7Defer);
    sysex8Builder.emplace (midiEndpointProperties.received, defer);
    mdsBuilder.emplace    (midiEndpointProperties.received, defer);
    textBuilder.emplace   (midiEndpointProperties.received, defer);

    midiEndpointProperties.playRequested.onPropertyChange (
        [this] (const juce::Identifier&)
        {
            if (midiEndpointProperties.playRequested.get ())
                startPlayback ();
        });

    runtimeContext.clearEvents.onPropertyChange ([this] (const juce::Identifier&)
    {
        midiEndpointProperties.received.clear ();
        midiEndpointProperties.transmitted.clear ();
        midiEndpointProperties.deferredMessages.clear ();
        startTime = -1;
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
        if (!midiProperties.pause.get () && !midiEndpointProperties.pause.get ())
        {
            midiEndpointProperties.received.addEvent (copy);
            midiEndpointProperties.drainDeferredMessages ();
        }

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
    addFlexDataTextEvents ();
    addMidi1Events ();
    addMidi2Events ();
    addStreamEvents ();
    addCiPeCapabilitiesEvents ();
    addCiPeGetPropertyDataEvents ();
    addCiPeSetPropertyDataEvents ();
    addCiPeSubscriptionEvents ();
    addCiPeNotifyEvent ();
    addCiProfileEvents ();
    addCiProfileSetEvents ();
    addCiProfileEnabledDisabledEvents ();
    addCiProfileReportEvents ();
    addCiProfileDetailsEvents ();
    addCiProfileSpecificDataEvent ();
    addCiDiscoveryEvents ();
    addCiEndpointEvents ();
    addCiInvalidateMuidEvent ();
    addCiNakEvent ();
    addCiAckEvent ();
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
    Sysex7EventFactory factory ([this] (Sysex7Event e)
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
    Sysex8EventFactory factory ([this] (Sysex8Event e)
                                { eventList.addEvent (100, e); });
    factory.createEvents (1, 0x04, std::span (payload));
}

void SyntheticEndpointController::addFlexDataTextEvents ()
{
    auto addText = [this] (FlexDataTextEvent e) { eventList.addEvent (100, e); };
    FlexDataTextEventFactory factory (addText);

    // Short — fits in one complete packet
    factory.createEvents (1, FlexDataStatusBank::metadataText,
                          static_cast<int> (MetadataTextStatus::composerName), "J.S. Bach");

    // Longer — spans three packets (start / continue / end); 26 chars = 12+12+2
    factory.createEvents (1, FlexDataStatusBank::metadataText,
                          static_cast<int> (MetadataTextStatus::compositionName),
                          "Brandenburg Concerto No. 3");

    // Arranger — single packet
    factory.createEvents (1, FlexDataStatusBank::metadataText,
                          static_cast<int> (MetadataTextStatus::arrangerName), "Ravel");

    // Performance text — single packet
    factory.createEvents (1, FlexDataStatusBank::performanceText,
                          static_cast<int> (PerformanceTextStatus::lyrics), "Twinkle");
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

    // No chord (clear)
    eventList.addEvent (100, SetChordEvent (1, 0, TonicNote::unknown, ChordType::noChord));
    // Bb Minor (spec example)
    eventList.addEvent (100, SetChordEvent (1, -1, TonicNote::b, ChordType::minor));
    // D Major / F# bass (spec example)
    {
        SetChordEvent e (1, 0, TonicNote::d, ChordType::major);
        e.bassSharpsFlats = 1;
        e.bassNote        = TonicNote::f;
        eventList.addEvent (100, e);
    }
    // C Major 7th with raised 11th (spec example CMaj7#11)
    {
        SetChordEvent e (1, 0, TonicNote::c, ChordType::major7th);
        e.alter1Type   = AlterationType::raise;
        e.alter1Degree = 11;
        eventList.addEvent (100, e);
    }
}

void SyntheticEndpointController::addMidi2Events ()
{
    eventList.addEvent (1000, Midi2NoteOnEvent  (1, 1, 62, MidiUnipolarFloat (0.8f)));
    eventList.addEvent (250,  Midi2NoteOffEvent (1, 1, 62, MidiUnipolarFloat (0.25f)));
}

void SyntheticEndpointController::addStreamEvents ()
{
    // Endpoint Discovery — request all notification types
    eventList.addEvent (100, EndpointDiscoveryEvent (1, 1, true, true, true, true, true));

    // Endpoint Info — 2 static function blocks, supports both MIDI 1.0 and 2.0
    eventList.addEvent (100, EndpointInfoNotificationEvent (1, 1, true, 2, true, true, false, false));

    // Device Identity — Yamaha (0x43), family 5, model 3, revision 1.0.0.0
    eventList.addEvent (100, DeviceIdentityNotificationEvent (0x43, 0, 0,
                                                               MidiWord (5), MidiWord (3),
                                                               1, 0, 0, 0));

    // Endpoint name (fits one packet) and product instance ID
    {
        auto add = [this] (StreamTextEvent e) { eventList.addEvent (100, e); };
        StreamTextEventFactory factory (add);
        factory.createEvents (StreamStatus::endpointNameNotification, 0, "Synth Piano");
        factory.createEvents (StreamStatus::productInstanceId,        0, "SN20250401");
    }

    // Stream configuration — request MIDI 2.0, no JR timestamps; device confirms
    eventList.addEvent (100, StreamConfigurationRequestEvent      (2, false, false));
    eventList.addEvent (100, StreamConfigurationNotificationEvent (2, false, false));

    // Function Block Discovery — all blocks, request info and name for each
    eventList.addEvent (100, FunctionBlockDiscoveryEvent (0xFF, true, true));

    // Function Block 0: Keys — bidirectional, MIDI 2.0, group 0, 16 SysEx8 streams
    //   uiHint=3 (sender+receiver), midi1=0 (no restriction), direction=3 (bidirectional)
    eventList.addEvent (100, FunctionBlockInfoNotificationEvent (true, 0, 3, 0, 3, 0, 1, 16, 16));

    // Function Block 1: Pads — input only, MIDI 2.0, group 1, 8 SysEx8 streams
    //   uiHint=2 (sender), midi1=0, direction=1 (input to MIDI device)
    eventList.addEvent (100, FunctionBlockInfoNotificationEvent (true, 1, 2, 0, 1, 1, 1, 16, 8));

    // Function block names
    {
        auto add = [this] (StreamTextEvent e) { eventList.addEvent (100, e); };
        StreamTextEventFactory factory (add);
        factory.createEvents (StreamStatus::functionBlockNameNotification, 0, "Keys");
        factory.createEvents (StreamStatus::functionBlockNameNotification, 1, "Pads");
    }

    // Clip markers
    eventList.addEvent (100, StartOfClipEvent ());
    eventList.addEvent (100, EndOfClipEvent   ());
}

void SyntheticEndpointController::addCiPeCapabilitiesEvents ()
{
    const MidiGroup group { 1 };
    const int initiatorMuid { 0x01234567 };
    const int responderMuid { 0x0654321 };

    auto addEvents = [this, &group] (auto& msg)
    {
        if (auto buf { msg.data.get () }; buf != nullptr)
        {
            Sysex7EventFactory factory ([this] (Sysex7Event e) { eventList.addEvent (100, e); });
            factory.createEvents (group, std::span<const uint8_t> (buf->cbegin (), buf->cend ()));
        }
    };

    // Initiator asks responder about PE capabilities (v2: major=1, minor=0)
    CiPeCapabilitiesInquiry inquiry { group, initiatorMuid, responderMuid, 3, 1, 0 };
    auto inquiryMsg { inquiry.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    addEvents (inquiryMsg);

    // Responder reports it supports 2 simultaneous requests, same PE version
    CiPeCapabilitiesReply reply { group, responderMuid, initiatorMuid, 2, 1, 0 };
    auto replyMsg { reply.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    addEvents (replyMsg);
}

void SyntheticEndpointController::addCiPeGetPropertyDataEvents ()
{
    const MidiGroup group { 1 };
    const int initiatorMuid { 0x01234567 };
    const int responderMuid { 0x0654321 };

    auto addEvents = [this, &group] (auto& msg)
    {
        if (auto buf { msg.data.get () }; buf != nullptr)
        {
            Sysex7EventFactory factory ([this] (Sysex7Event e) { eventList.addEvent (100, e); });
            factory.createEvents (group, std::span<const uint8_t> (buf->cbegin (), buf->cend ()));
        }
    };

    // Inquiry: request the value of the "X-DeviceName" resource
    const juce::String headerJson { R"({"resource":"X-DeviceName"})" };
    Buffer::Ptr hdrBuf = new Buffer ();
    for (auto c : headerJson)
        hdrBuf->append (static_cast<uint8_t> (c));

    CiPeGetPropertyDataInquiry inquiry { group, initiatorMuid, responderMuid, 1, hdrBuf };
    auto inquiryMsg { inquiry.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    addEvents (inquiryMsg);

    // Reply: property value is "Synth Piano"
    const juce::String replyHeader { R"({"status":200})" };
    Buffer::Ptr rHdr = new Buffer ();
    for (auto c : replyHeader)
        rHdr->append (static_cast<uint8_t> (c));

    const juce::String propValue { R"("Synth Piano")" };
    Buffer::Ptr propBuf = new Buffer ();
    for (auto c : propValue)
        propBuf->append (static_cast<uint8_t> (c));

    CiPeGetPropertyDataReply reply { group, responderMuid, initiatorMuid,
                                     1, rHdr, 1, 1, propBuf };
    auto replyMsg { reply.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    addEvents (replyMsg);
}

void SyntheticEndpointController::addCiPeSetPropertyDataEvents ()
{
    const MidiGroup group { 1 };
    const int initiatorMuid { 0x01234567 };
    const int responderMuid { 0x0654321 };

    auto addEvents = [this, &group] (auto& msg)
    {
        if (auto buf { msg.data.get () }; buf != nullptr)
        {
            Sysex7EventFactory factory ([this] (Sysex7Event e) { eventList.addEvent (100, e); });
            factory.createEvents (group, std::span<const uint8_t> (buf->cbegin (), buf->cend ()));
        }
    };

    // Inquiry: write a new value to "X-DeviceName"
    const juce::String setHeader { R"({"resource":"X-DeviceName","command":"full"})" };
    Buffer::Ptr sHdr = new Buffer ();
    for (auto c : setHeader)
        sHdr->append (static_cast<uint8_t> (c));

    const juce::String newValue { R"("Grand Piano")" };
    Buffer::Ptr propBuf = new Buffer ();
    for (auto c : newValue)
        propBuf->append (static_cast<uint8_t> (c));

    CiPeSetPropertyDataInquiry inquiry { group, initiatorMuid, responderMuid,
                                         2, sHdr, 1, 1, propBuf };
    auto inquiryMsg { inquiry.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    addEvents (inquiryMsg);

    // Reply: status 200 OK, no property data
    const juce::String replyHeader { R"({"status":200})" };
    Buffer::Ptr rHdr = new Buffer ();
    for (auto c : replyHeader)
        rHdr->append (static_cast<uint8_t> (c));

    CiPeSetPropertyDataReply reply { group, responderMuid, initiatorMuid, 2, rHdr };
    auto replyMsg { reply.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    addEvents (replyMsg);
}

void SyntheticEndpointController::addCiPeSubscriptionEvents ()
{
    const MidiGroup group { 1 };
    const int initiatorMuid { 0x01234567 };
    const int responderMuid { 0x0654321 };

    auto addEvents = [this, &group] (auto& msg)
    {
        if (auto buf { msg.data.get () }; buf != nullptr)
        {
            Sysex7EventFactory factory ([this] (Sysex7Event e) { eventList.addEvent (100, e); });
            factory.createEvents (group, std::span<const uint8_t> (buf->cbegin (), buf->cend ()));
        }
    };

    // Inquiry: subscribe to "X-State" property updates
    const juce::String subHeader { R"({"resource":"X-State","command":"start"})" };
    Buffer::Ptr sHdr = new Buffer ();
    for (auto c : subHeader)
        sHdr->append (static_cast<uint8_t> (c));

    CiPeSubscriptionInquiry inquiry { group, initiatorMuid, responderMuid,
                                      3, sHdr, 1, 1, {} };
    auto inquiryMsg { inquiry.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    addEvents (inquiryMsg);

    // Reply: subscription confirmed, current value included
    const juce::String replyHeader { R"({"status":200,"command":"start"})" };
    Buffer::Ptr rHdr = new Buffer ();
    for (auto c : replyHeader)
        rHdr->append (static_cast<uint8_t> (c));

    const juce::String stateValue { R"({"active":true})" };
    Buffer::Ptr propBuf = new Buffer ();
    for (auto c : stateValue)
        propBuf->append (static_cast<uint8_t> (c));

    CiPeSubscriptionReply reply { group, responderMuid, initiatorMuid,
                                   3, rHdr, 1, 1, propBuf };
    auto replyMsg { reply.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    addEvents (replyMsg);
}

void SyntheticEndpointController::addCiPeNotifyEvent ()
{
    // Notify (0x3F) is deprecated; emitted here only to exercise legacy-device parsing.
    const MidiGroup group { 1 };
    const int responderMuid { 0x0654321 };
    const int initiatorMuid { 0x01234567 };

    const juce::String notifyHeader { R"({"status":144})" }; // 144 = 0x90, example error code
    Buffer::Ptr hdr = new Buffer ();
    for (auto c : notifyHeader)
        hdr->append (static_cast<uint8_t> (c));

    CiPeNotify msg { group, responderMuid, initiatorMuid, 1, hdr, 1, 1, {} };
    auto sysexMsg { msg.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    if (auto buf { sysexMsg.data.get () }; buf != nullptr)
    {
        Sysex7EventFactory factory ([this] (Sysex7Event e) { eventList.addEvent (100, e); });
        factory.createEvents (group, std::span<const uint8_t> (buf->cbegin (), buf->cend ()));
    }
}

void SyntheticEndpointController::addCiProfileEvents ()
{
    const MidiGroup group { 1 };
    const int initiatorMuid { 0x01234567 };
    const int responderMuid { 0x0654321 };

    auto addEvents = [this, &group] (auto& msg)
    {
        if (auto buf { msg.data.get () }; buf != nullptr)
        {
            Sysex7EventFactory factory ([this] (Sysex7Event e) { eventList.addEvent (100, e); });
            factory.createEvents (group, std::span<const uint8_t> (buf->cbegin (), buf->cend ()));
        }
    };

    // Inquiry: initiator asks responder which profiles it supports
    CiProfileInquiry inquiry { group, initiatorMuid, responderMuid };
    auto inquiryMsg { inquiry.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    addEvents (inquiryMsg);

    // Reply: two enabled profiles (one standard, one manufacturer), one disabled
    CiProfileInquiryReply reply { group, responderMuid, initiatorMuid };
    reply.addEnabledProfile  ({ 0x7E, 0x00, 0x21, 0x01, 0x01 }); // Standard: Piano (bank 0, #0x21, v1, lvl1)
    reply.addEnabledProfile  ({ 0x41, 0x00, 0x00, 0x01, 0x00 }); // Manufacturer: Roland
    reply.addDisabledProfile ({ 0x7E, 0x00, 0x24, 0x01, 0x01 }); // Standard: Drawbar Organ (bank 0, #0x24, v1, lvl1)
    auto replyMsg { reply.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    addEvents (replyMsg);
}

void SyntheticEndpointController::addCiProfileSetEvents ()
{
    const MidiGroup group { 1 };
    const int initiatorMuid { 0x01234567 };
    const int responderMuid { 0x0654321 };
    const ProfileId profile { 0x7E, 0x00, 0x21, 0x01, 0x01 }; // Standard Piano profile

    auto addEvents = [this, &group] (auto& msg)
    {
        if (auto buf { msg.data.get () }; buf != nullptr)
        {
            Sysex7EventFactory factory ([this] (Sysex7Event e) { eventList.addEvent (100, e); });
            factory.createEvents (group, std::span<const uint8_t> (buf->cbegin (), buf->cend ()));
        }
    };

    // Initiator enables the Piano profile on 2 channels
    CiProfileSetOn setOn { group, initiatorMuid, responderMuid, profile, 2 };
    auto setOnMsg { setOn.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    addEvents (setOnMsg);

    // Initiator disables the same profile
    CiProfileSetOff setOff { group, initiatorMuid, responderMuid, profile };
    auto setOffMsg { setOff.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    addEvents (setOffMsg);
}

void SyntheticEndpointController::addCiProfileDetailsEvents ()
{
    const MidiGroup group { 1 };
    const int initiatorMuid { 0x01234567 };
    const int responderMuid { 0x0654321 };
    const ProfileId profile { 0x7E, 0x00, 0x21, 0x01, 0x01 }; // Standard Piano profile

    auto addEvents = [this, &group] (auto& msg)
    {
        if (auto buf { msg.data.get () }; buf != nullptr)
        {
            Sysex7EventFactory factory ([this] (Sysex7Event e) { eventList.addEvent (100, e); });
            factory.createEvents (group, std::span<const uint8_t> (buf->cbegin (), buf->cend ()));
        }
    };

    // Inquiry: initiator asks for registered target data (0x01)
    CiProfileDetailsInquiry inquiry { group, initiatorMuid, responderMuid,
                                      profile, MidiByte { 0x01 } };
    auto inquiryMsg { inquiry.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    addEvents (inquiryMsg);

    // Reply: responder provides 4 bytes of target data
    Buffer::Ptr data = new Buffer ();
    data->append (0x01);
    data->append (0x00);
    data->append (0x10);
    data->append (0x00);
    CiProfileDetailsInquiryReply reply { group, responderMuid, initiatorMuid,
                                         profile, MidiByte { 0x01 }, data };
    auto replyMsg { reply.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    addEvents (replyMsg);
}

void SyntheticEndpointController::addCiProfileEnabledDisabledEvents ()
{
    const MidiGroup group { 1 };
    const int responderMuid { 0x0654321 };
    const ProfileId profile { 0x7E, 0x00, 0x21, 0x01, 0x01 }; // Standard Piano profile

    auto addEvents = [this, &group] (auto& msg)
    {
        if (auto buf { msg.data.get () }; buf != nullptr)
        {
            Sysex7EventFactory factory ([this] (Sysex7Event e) { eventList.addEvent (100, e); });
            factory.createEvents (group, std::span<const uint8_t> (buf->cbegin (), buf->cend ()));
        }
    };

    // Responder reports Piano profile is now enabled on 2 channels
    CiProfileEnabled enabled { group, responderMuid, profile, 2 };
    auto enabledMsg { enabled.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    addEvents (enabledMsg);

    // Responder reports Piano profile is now disabled on those channels
    CiProfileDisabled disabled { group, responderMuid, profile, 2 };
    auto disabledMsg { disabled.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    addEvents (disabledMsg);
}

void SyntheticEndpointController::addCiProfileReportEvents ()
{
    const MidiGroup group { 1 };
    const int responderMuid { 0x0654321 };

    auto addEvents = [this, &group] (auto& msg)
    {
        if (auto buf { msg.data.get () }; buf != nullptr)
        {
            Sysex7EventFactory factory ([this] (Sysex7Event e) { eventList.addEvent (100, e); });
            factory.createEvents (group, std::span<const uint8_t> (buf->cbegin (), buf->cend ()));
        }
    };

    // Responder announces a newly available standard profile.
    CiProfileAdded added { group, responderMuid, { 0x7E, 0x00, 0x2B, 0x01, 0x01 } };
    auto addedMsg { added.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    addEvents (addedMsg);

    // Responder announces a profile it no longer supports.
    CiProfileRemoved removed { group, responderMuid, { 0x7E, 0x00, 0x21, 0x01, 0x01 } };
    auto removedMsg { removed.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    addEvents (removedMsg);
}

void SyntheticEndpointController::addCiProfileSpecificDataEvent ()
{
    const MidiGroup group { 1 };
    const int initiatorMuid { 0x01234567 };
    const int responderMuid { 0x0654321 };
    const ProfileId profile { 0x7E, 0x00, 0x21, 0x01, 0x01 }; // Standard Piano profile

    Buffer::Ptr data = new Buffer ();
    data->append (0x01);
    data->append (0x02);
    data->append (0x03);
    data->append (0x04);
    data->append (0x05);
    data->append (0x06);

    CiProfileSpecificData msg { group, initiatorMuid, responderMuid, profile, data };
    auto sysexMsg { msg.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    if (auto buf { sysexMsg.data.get () }; buf != nullptr)
    {
        Sysex7EventFactory factory ([this] (Sysex7Event e) { eventList.addEvent (100, e); });
        factory.createEvents (group, std::span<const uint8_t> (buf->cbegin (), buf->cend ()));
    }
}

void SyntheticEndpointController::addCiDiscoveryEvents ()
{
    const MidiGroup group { 1 };
    const int initiatorMuid { 0x01234567 };
    const int responderMuid { 0x0654321 };

    ManufacturerId manufacturer { MidiByte { 0x41 }, MidiByte { 0x00 }, MidiByte { 0x00 } }; // Roland
    DeviceFamily      family    { MidiByte { 0x42 }, MidiByte { 0x00 } };
    DeviceFamilyModel model     { MidiByte { 0x12 }, MidiByte { 0x00 } };
    const int categories { CiCategory::profileConfiguration | CiCategory::propertyExchange };

    // Inquiry — broadcast (format 1)
    CiDiscoveryInquiry inquiry { group, initiatorMuid, manufacturer, family, model,
                                 MidiByte { 1 }, MidiByte { 2 }, MidiByte { 0 }, MidiByte { 0 },
                                 categories, MidiLong { 512 } };
    auto inquiryMsg { inquiry.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    if (auto buf { inquiryMsg.data.get () }; buf != nullptr)
    {
        Sysex7EventFactory factory ([this] (Sysex7Event e) { eventList.addEvent (100, e); });
        factory.createEvents (group, std::span<const uint8_t> (buf->cbegin (), buf->cend ()));
    }

    // Reply — unicast back to initiator (format 1)
    CiDiscoveryReply reply { group, responderMuid, initiatorMuid, manufacturer, family, model,
                             MidiByte { 3 }, MidiByte { 1 }, MidiByte { 0 }, MidiByte { 0 },
                             categories, MidiLong { 512 } };
    auto replyMsg { reply.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    if (auto buf { replyMsg.data.get () }; buf != nullptr)
    {
        Sysex7EventFactory factory ([this] (Sysex7Event e) { eventList.addEvent (100, e); });
        factory.createEvents (group, std::span<const uint8_t> (buf->cbegin (), buf->cend ()));
    }
}

void SyntheticEndpointController::addCiEndpointEvents ()
{
    const MidiGroup group { 1 };
    const int initiatorMuid { 0x01234567 };
    const int responderMuid { 0x0654321 };

    auto addEvents = [this, &group] (auto& msg)
    {
        if (auto buf { msg.data.get () }; buf != nullptr)
        {
            Sysex7EventFactory factory ([this] (Sysex7Event e) { eventList.addEvent (100, e); });
            factory.createEvents (group, std::span<const uint8_t> (buf->cbegin (), buf->cend ()));
        }
    };

    // Inquiry: initiator asks responder for its Product Instance ID
    CiEndpointInquiry inquiry { group, initiatorMuid, responderMuid,
                                MidiByte { CiEndpointStatus::productInstanceId } };
    auto inquiryMsg { inquiry.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    addEvents (inquiryMsg);

    // Reply: responder provides its Product Instance ID
    CiEndpointReply reply { group, responderMuid, initiatorMuid,
                            MidiByte { CiEndpointStatus::productInstanceId },
                            "SN20250401-SYNTH" };
    auto replyMsg { reply.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    addEvents (replyMsg);
}

void SyntheticEndpointController::addCiInvalidateMuidEvent ()
{
    const MidiGroup group { 1 };
    const int initiatorMuid { 0x01234567 };
    const int responderMuid { 0x0654321 };

    CiInvalidateMuid msg { group, initiatorMuid, responderMuid };
    auto sysexMsg { msg.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    if (auto buf { sysexMsg.data.get () }; buf != nullptr)
    {
        Sysex7EventFactory factory ([this] (Sysex7Event e) { eventList.addEvent (100, e); });
        factory.createEvents (group, std::span<const uint8_t> (buf->cbegin (), buf->cend ()));
    }
}

void SyntheticEndpointController::addCiNakEvent ()
{
    const MidiGroup group { 1 };
    const int initiatorMuid { 0x01234567 };
    const int responderMuid { 0x0654321 };

    // NAK from responder: profile configuration is not supported.
    std::array<uint8_t, CiNak::kDetailBytes> details { 0, 0, 0, 0, 0 };
    CiNak msg { group, responderMuid, initiatorMuid,
                MidiByte { CiType::discoveryInquiry },
                MidiByte { CiNakStatus::messageNotSupported },
                MidiByte { 0 },
                details,
                "Discovery not supported on this device" };
    auto sysexMsg { msg.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    if (auto buf { sysexMsg.data.get () }; buf != nullptr)
    {
        Sysex7EventFactory factory ([this] (Sysex7Event e) { eventList.addEvent (100, e); });
        factory.createEvents (group, std::span<const uint8_t> (buf->cbegin (), buf->cend ()));
    }
}

void SyntheticEndpointController::addCiAckEvent ()
{
    const MidiGroup group { 1 };
    const int initiatorMuid { 0x01234567 };
    const int responderMuid { 0x0654321 };

    // ACK from responder confirming the Discovery Inquiry was received.
    std::array<uint8_t, CiAck::kDetailBytes> details { 0, 0, 0, 0, 0 };
    CiAck msg { group, responderMuid, initiatorMuid,
                MidiByte { CiType::discoveryInquiry },
                MidiByte { CiAckStatus::ack },
                MidiByte { 0 },
                details,
                "Discovery acknowledged" };
    auto sysexMsg { msg.toSysex7Message (MidiNibble { group }, messageFormatLatest) };
    if (auto buf { sysexMsg.data.get () }; buf != nullptr)
    {
        Sysex7EventFactory factory ([this] (Sysex7Event e) { eventList.addEvent (100, e); });
        factory.createEvents (group, std::span<const uint8_t> (buf->cbegin (), buf->cend ()));
    }
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
        [this] (MixedDataSetHeaderEvent h)  { eventList.addEvent (100, h); },
        [this] (MixedDataSetPayloadEvent p) { eventList.addEvent (100, p); });
    factory.createEvents (1, 2, 0x0041, 0x0010, 0x02, 0x03, std::span (payload));
}
