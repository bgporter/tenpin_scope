
#include <juce_core/juce_core.h>

class Test_FlexData : public TestSuite
{
public:
    Test_FlexData ()
    : TestSuite ("FlexData", "!!! category !!!")
    {
    }

    void runTest () override
    {
        using namespace midi_literals;

        test ("bpmToTenNs: 120 BPM", [&] () { expect (bpmToTenNs (120.0) == 50'000'000u); });

        test ("bpmToTenNs: 60 BPM", [&] () { expect (bpmToTenNs (60.0) == 100'000'000u); });

        test ("tenNsToBpm: roundtrip 120 BPM",
              [&] ()
              {
                  const double result = tenNsToBpm (bpmToTenNs (120.0));
                  expect (std::abs (result - 120.0) < 0.001);
              });

        test ("tenNsToBpm: roundtrip 93.5 BPM",
              [&] ()
              {
                  const double result = tenNsToBpm (bpmToTenNs (93.5));
                  expect (std::abs (result - 93.5) < 0.01);
              });

        test ("SetTempoEvent construction at 120 BPM",
              [&] ()
              {
                  SetTempoEvent e (2_gr, 120.0);
                  expect (e.userGroup == 2);
                  expect (e.format == FlexDataFormat::complete);
                  expect (e.address == FlexDataAddress::group);
                  expect (e.channel == 0);
                  expect (e.statusBank == FlexDataStatusBank::setupAndPerformance);
                  expect (e.status == static_cast<int> (SetupAndPerformanceStatus::setTempo));
                  expect (e.tenNsPerQuarterNote == 50'000'000u);
              });

        test ("SetTempoEvent construction at 60 BPM",
              [&] ()
              {
                  SetTempoEvent e (1_gr, 60.0);
                  expect (e.tenNsPerQuarterNote == 100'000'000u);
              });

        test ("SetTempoEvent messageType is flexData",
              [&] ()
              {
                  SetTempoEvent e (1_gr, 120.0);
                  expect (e.messageType == MessageTypes::flexData);
              });

        test ("SetTempoEvent ValueTree roundtrip",
              [&] ()
              {
                  SetTempoEvent e (3_gr, 120.0);
                  juce::ValueTree vt = e;
                  expect (vt.isValid ());
                  UmpEvent ump (vt);
                  SetTempoEvent e2 (ump);
                  expect (e2.userGroup == 3);
                  expect (e2.format == FlexDataFormat::complete);
                  expect (e2.address == FlexDataAddress::group);
                  expect (e2.statusBank == FlexDataStatusBank::setupAndPerformance);
                  expect (e2.status == static_cast<int> (SetupAndPerformanceStatus::setTempo));
                  expect (e2.tenNsPerQuarterNote == 50'000'000u);
              });

        test ("FlexDataEvent base: Word 0 fields parse correctly",
              [&] ()
              {
                  SetTempoEvent src (1_gr, 120.0);
                  juce::ValueTree vt = src;
                  UmpEvent ump (vt);
                  FlexDataEvent e (ump);
                  expect (e.userGroup == 1);
                  expect (e.format == FlexDataFormat::complete);
                  expect (e.address == FlexDataAddress::group);
                  expect (e.channel == 0);
                  expect (e.statusBank == FlexDataStatusBank::setupAndPerformance);
                  expect (e.status == static_cast<int> (SetupAndPerformanceStatus::setTempo));
              });

        test ("SetTimeSignatureEvent construction",
              [&] ()
              {
                  SetTimeSignatureEvent e (2_gr, 4, 2, 8);
                  expect (e.userGroup == 2);
                  expect (e.format == FlexDataFormat::complete);
                  expect (e.address == FlexDataAddress::group);
                  expect (e.statusBank == FlexDataStatusBank::setupAndPerformance);
                  expect (e.status == static_cast<int> (SetupAndPerformanceStatus::setTimeSignature));
                  expect (e.numerator == 4);
                  expect (e.denominatorPower == 2);
                  expect (e.num32ndNotes == 8);
              });

        test ("SetTimeSignatureEvent: 6/8",
              [&] ()
              {
                  SetTimeSignatureEvent e (1_gr, 6, 3, 8);
                  expect (e.numerator == 6);
                  expect (e.denominatorPower == 3);
                  expect (e.num32ndNotes == 8);
              });

        test ("SetTimeSignatureEvent ValueTree roundtrip",
              [&] ()
              {
                  SetTimeSignatureEvent e (1_gr, 4, 2, 8);
                  juce::ValueTree vt = e;
                  UmpEvent ump (vt);
                  SetTimeSignatureEvent e2 (ump);
                  expect (e2.userGroup == 1);
                  expect (e2.numerator == 4);
                  expect (e2.denominatorPower == 2);
                  expect (e2.num32ndNotes == 8);
                  expect (e2.status == static_cast<int> (SetupAndPerformanceStatus::setTimeSignature));
              });

        test ("SetMetronomeEvent construction",
              [&] ()
              {
                  SetMetronomeEvent e (1_gr, 24, 4, 0, 0, 2, 0);
                  expect (e.userGroup == 1);
                  expect (e.format == FlexDataFormat::complete);
                  expect (e.address == FlexDataAddress::group);
                  expect (e.statusBank == FlexDataStatusBank::setupAndPerformance);
                  expect (e.status == static_cast<int> (SetupAndPerformanceStatus::setMetronome));
                  expect (e.numClocksPerPrimaryClick == 24);
                  expect (e.barAccentPart1 == 4);
                  expect (e.barAccentPart2 == 0);
                  expect (e.barAccentPart3 == 0);
                  expect (e.numSubdivisionClicks1 == 2);
                  expect (e.numSubdivisionClicks2 == 0);
              });

        test ("SetMetronomeEvent: 5/4 (3+2) with subdivisions",
              [&] ()
              {
                  SetMetronomeEvent e (1_gr, 12, 3, 2, 0, 3, 2);
                  expect (e.numClocksPerPrimaryClick == 12);
                  expect (e.barAccentPart1 == 3);
                  expect (e.barAccentPart2 == 2);
                  expect (e.barAccentPart3 == 0);
                  expect (e.numSubdivisionClicks1 == 3);
                  expect (e.numSubdivisionClicks2 == 2);
              });

        test ("SetKeySignatureEvent: sharpsFlats sign extension",
              [&] ()
              {
                  // Verify the computed member sign-extends the raw 4-bit value correctly.
                  // We test by constructing events with known signed values and reading back.
                  expect (SetKeySignatureEvent (1_gr, 0, TonicNote::c).sharpsFlats == 0);
                  expect (SetKeySignatureEvent (1_gr, 7, TonicNote::g).sharpsFlats == 7);
                  expect (SetKeySignatureEvent (1_gr, -1, TonicNote::f).sharpsFlats == -1);
                  expect (SetKeySignatureEvent (1_gr, -7, TonicNote::a).sharpsFlats == -7);
                  expect (SetKeySignatureEvent (1_gr, -8, TonicNote::unknown).sharpsFlats == -8);
              });

        test ("SetKeySignatureEvent construction: C major",
              [&] ()
              {
                  SetKeySignatureEvent e (2_gr, 0, TonicNote::c);
                  expect (e.userGroup == 2);
                  expect (e.format == FlexDataFormat::complete);
                  expect (e.address == FlexDataAddress::group);
                  expect (e.statusBank == FlexDataStatusBank::setupAndPerformance);
                  expect (e.status == static_cast<int> (SetupAndPerformanceStatus::setKeySignature));
                  expect (e.sharpsFlats == 0);
                  expect (e.tonicNote == TonicNote::c);
              });

        test ("SetKeySignatureEvent construction: G major (1 sharp)",
              [&] ()
              {
                  SetKeySignatureEvent e (1_gr, 1, TonicNote::g);
                  expect (e.sharpsFlats == 1);
                  expect (e.tonicNote == TonicNote::g);
              });

        test ("SetKeySignatureEvent construction: Bb major (2 flats)",
              [&] ()
              {
                  SetKeySignatureEvent e (1_gr, -2, TonicNote::b);
                  expect (e.sharpsFlats == -2);
                  expect (e.tonicNote == TonicNote::b);
              });

        test ("SetKeySignatureEvent construction: unknown key",
              [&] ()
              {
                  SetKeySignatureEvent e (1_gr, -8, TonicNote::unknown);
                  expect (e.sharpsFlats == -8);
                  expect (e.tonicNote == TonicNote::unknown);
              });

        test ("SetKeySignatureEvent ValueTree roundtrip",
              [&] ()
              {
                  SetKeySignatureEvent e (3_gr, -3, TonicNote::e);
                  juce::ValueTree vt = e;
                  UmpEvent ump (vt);
                  SetKeySignatureEvent e2 (ump);
                  expect (e2.userGroup == 3);
                  expect (e2.sharpsFlats == -3);
                  expect (e2.tonicNote == TonicNote::e);
                  expect (e2.statusBank == FlexDataStatusBank::setupAndPerformance);
                  expect (e2.status == static_cast<int> (SetupAndPerformanceStatus::setKeySignature));
              });

        test ("SetMetronomeEvent ValueTree roundtrip",
              [&] ()
              {
                  SetMetronomeEvent e (2_gr, 12, 3, 2, 0, 3, 2);
                  juce::ValueTree vt = e;
                  UmpEvent ump (vt);
                  SetMetronomeEvent e2 (ump);
                  expect (e2.userGroup == 2);
                  expect (e2.numClocksPerPrimaryClick == 12);
                  expect (e2.barAccentPart1 == 3);
                  expect (e2.barAccentPart2 == 2);
                  expect (e2.barAccentPart3 == 0);
                  expect (e2.numSubdivisionClicks1 == 3);
                  expect (e2.numSubdivisionClicks2 == 2);
              });

        test ("SetChordEvent: no chord",
              [&] ()
              {
                  SetChordEvent e (1_gr, 0, TonicNote::unknown, ChordType::noChord);
                  expect (e.chordType.get () == ChordType::noChord);
                  expect (e.chordTonic.get () == TonicNote::unknown);
                  expect (e.messageType == MessageTypes::flexData);
              });

        test ("SetChordEvent: BbMin construction",
              [&] ()
              {
                  SetChordEvent e (1_gr, -1, TonicNote::b, ChordType::minor);
                  expect (e.chordSharpsFlats == -1);
                  expect (e.chordTonic.get () == TonicNote::b);
                  expect (e.chordType.get () == ChordType::minor);
                  expect (e.statusBank.get () == FlexDataStatusBank::setupAndPerformance);
                  expect (e.status == static_cast<int> (SetupAndPerformanceStatus::setChord));
                  expect (e.messageType == MessageTypes::flexData);
              });

        test ("SetChordEvent: chordSharpsFlats sign extension",
              [&] ()
              {
                  expect (SetChordEvent (1_gr, 0, TonicNote::c, ChordType::major).chordSharpsFlats == 0);
                  expect (SetChordEvent (1_gr, 2, TonicNote::c, ChordType::major).chordSharpsFlats == 2);
                  expect (SetChordEvent (1_gr, -1, TonicNote::b, ChordType::minor).chordSharpsFlats == -1);
                  expect (SetChordEvent (1_gr, -2, TonicNote::b, ChordType::minor).chordSharpsFlats == -2);
              });

        test ("SetChordEvent: bassSharpsFlats sentinel and sign extension",
              [&] ()
              {
                  SetChordEvent e (1_gr, 0, TonicNote::c, ChordType::major);
                  expect (e.bassSharpsFlats == -8); // default: same as chord tonic

                  e.bassSharpsFlats = 1;
                  expect (e.bassSharpsFlats == 1);

                  e.bassSharpsFlats = -2;
                  expect (e.bassSharpsFlats == -2);
              });

        test ("SetChordEvent: alterations",
              [&] ()
              {
                  SetChordEvent e (1_gr, 0, TonicNote::c, ChordType::major7th);
                  e.alter1Type   = AlterationType::raise;
                  e.alter1Degree = 11;
                  expect (e.alter1Type.get () == AlterationType::raise);
                  expect (e.alter1Degree == 11);
                  expect (e.alter2Type.get () == AlterationType::none);
              });

        test ("SetChordEvent: bass fields",
              [&] ()
              {
                  SetChordEvent e (1_gr, 0, TonicNote::d, ChordType::major);
                  e.bassSharpsFlats = 1;
                  e.bassNote        = TonicNote::f;
                  expect (e.bassSharpsFlats == 1);
                  expect (e.bassNote.get () == TonicNote::f);
                  expect (e.bassChordType.get () == ChordType::noChord);
              });

        test ("FlexDataTextEventFactory: complete packet for short text",
              [&] ()
              {
                  std::vector<FlexDataTextEvent> packets;
                  FlexDataTextEventFactory factory ([&] (FlexDataTextEvent e) { packets.push_back (e); });
                  factory.createEvents (1_gr, FlexDataStatusBank::metadataText,
                                        static_cast<int> (MetadataTextStatus::composerName), "Bach");
                  expect (packets.size () == 1u);
                  expect (packets[0].format.get () == FlexDataFormat::complete);
              });

        test ("FlexDataTextEvent: byte access",
              [&] ()
              {
                  std::vector<FlexDataTextEvent> packets;
                  FlexDataTextEventFactory factory ([&] (FlexDataTextEvent e) { packets.push_back (e); });
                  factory.createEvents (1_gr, FlexDataStatusBank::metadataText,
                                        static_cast<int> (MetadataTextStatus::composerName), "Hello");
                  const auto& e = packets[0];
                  expect (e[0] == 'H');
                  expect (e[1] == 'e');
                  expect (e[2] == 'l');
                  expect (e[3] == 'l');
                  expect (e[4] == 'o');
                  expect (e[5] == 0);
              });

        test ("FlexDataTextEvent: event name from status",
              [&] ()
              {
                  std::vector<FlexDataTextEvent> packets;
                  FlexDataTextEventFactory factory ([&] (FlexDataTextEvent e) { packets.push_back (e); });
                  factory.createEvents (1_gr, FlexDataStatusBank::metadataText,
                                        static_cast<int> (MetadataTextStatus::arrangerName), "x");
                  expect (packets[0].eventName == juce::String ("Flex Data: Arranger Name"));

                  packets.clear ();
                  factory.createEvents (1_gr, FlexDataStatusBank::performanceText,
                                        static_cast<int> (PerformanceTextStatus::lyrics), "x");
                  expect (packets[0].eventName == juce::String ("Flex Data: Lyrics"));
              });

        test ("FlexDataTextEventFactory: multi-packet for long text",
              [&] ()
              {
                  std::vector<FlexDataTextEvent> packets;
                  FlexDataTextEventFactory factory (
                      [&] (FlexDataTextEvent e)
                      {
                          DBG (e.toXmlString ());
                          DBG (static_cast<int> (e.format.get ()));
                          packets.push_back (e);
                      });
                  // 25 chars -> ceil(25/12) = 3 packets
                  factory.createEvents (1_gr, FlexDataStatusBank::metadataText,
                                        static_cast<int> (MetadataTextStatus::compositionName),
                                        "Brandenburg Concerto No 3");
                  expect (packets.size () == 3u);

                  expect (packets[0].format.get () == FlexDataFormat::start);
                  expect (packets[1].format.get () == FlexDataFormat::continue_);
                  expect (packets[2].format.get () == FlexDataFormat::end);
                  // First char of each packet
                  expect (packets[0][0] == 'B');
                  expect (packets[1][0] == 'C'); // byte 12 of "Brandenburg Concerto No 3"
                  expect (packets[2][0] == '3'); // byte 24 = last char
              });

        test ("FlexDataTextEvent: ValueTree roundtrip",
              [&] ()
              {
                  std::vector<FlexDataTextEvent> packets;
                  FlexDataTextEventFactory factory ([&] (FlexDataTextEvent e) { packets.push_back (e); });
                  factory.createEvents (1_gr, FlexDataStatusBank::performanceText,
                                        static_cast<int> (PerformanceTextStatus::lyrics), "Twinkle");
                  const auto& src    = packets[0];
                  juce::ValueTree vt = src;
                  UmpEvent ump (vt);
                  FlexDataTextEvent e2 (ump);
                  expect (e2.format.get () == FlexDataFormat::complete);
                  expect (e2.eventName == juce::String ("Flex Data: Lyrics"));
                  for (int i = 0; i < 7; ++i)
                      expect (e2[i] == src[i]);
                  expect (e2[7] == 0);
              });

        test ("SetChordEvent: ValueTree roundtrip (DMaj/F#)",
              [&] ()
              {
                  SetChordEvent src (1_gr, 0, TonicNote::d, ChordType::major);
                  src.bassSharpsFlats = 1;
                  src.bassNote        = TonicNote::f;
                  juce::ValueTree vt  = src;
                  UmpEvent ump (vt);
                  SetChordEvent e2 (ump);
                  expect (e2.chordSharpsFlats == 0);
                  expect (e2.chordTonic.get () == TonicNote::d);
                  expect (e2.chordType.get () == ChordType::major);
                  expect (e2.bassSharpsFlats == 1);
                  expect (e2.bassNote.get () == TonicNote::f);
                  expect (e2.bassChordType.get () == ChordType::noChord);
                  expect (e2.alter1Type.get () == AlterationType::none);
              });
    }
};

static Test_FlexData testFlexData;
