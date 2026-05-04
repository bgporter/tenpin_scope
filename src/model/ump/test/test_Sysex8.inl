

#include <juce_core/juce_core.h>

class Test_Sysex8 : public TestSuite
{
public:
    Test_Sysex8 ()
    : TestSuite ("Sysex8", "!!! category !!!")
    {
    }

    void runTest () override
    {
        using namespace midi_literals;

        test ("span construction",
              [&] ()
              {
                  const uint8_t buf[] = { 0x41, 0x10, 0x42, 0x12 };
                  Sysex8Event event (2_gr, SysexStatus::start, 0xAB, std::span (buf));
                  expect (event.userGroup == 2);
                  expect (event.status == SysexStatus::start);
                  expect (event.streamId == 0xAB);
                  expect (event.numBytes == 5); // 4 data bytes + 1 for stream ID
                  expect (event[0] == 0x41);
                  expect (event[1] == 0x10);
                  expect (event[2] == 0x42);
                  expect (event[3] == 0x12);
              });

        test ("empty span → numBytes == 1 (stream ID only)",
              [&] ()
              {
                  Sysex8Event event (1_gr, SysexStatus::complete, 0x01, std::span<const uint8_t> {});
                  expect (event.numBytes == 1);
                  expect (event.streamId == 0x01);
              });

        test ("13-byte span → numBytes == 14",
              [&] ()
              {
                  const uint8_t buf[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
                  Sysex8Event event (1_gr, SysexStatus::complete, 0x00, std::span (buf));
                  expect (event.numBytes == 14);
                  expect (event[12] == 13);
              });

        test ("span clamped to 13 bytes",
              [&] ()
              {
                  const uint8_t buf[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
                  Sysex8Event event (1_gr, SysexStatus::complete, 0x00, std::span (buf));
                  expect (event.numBytes == 14);
                  expect (event[12] == 13);
              });

        test ("full 8-bit data preserved (no 7-bit masking)",
              [&] ()
              {
                  const uint8_t buf[] = { 0xFF, 0x80, 0xC0 };
                  Sysex8Event event (1_gr, SysexStatus::complete, 0x00, std::span (buf));
                  expect (event[0] == 0xFF);
                  expect (event[1] == 0x80);
                  expect (event[2] == 0xC0);
              });

        test ("all status values",
              [&] ()
              {
                  const uint8_t buf[] = { 0x01 };
                  expect (Sysex8Event (1_gr, SysexStatus::complete,  0, std::span (buf)).status == SysexStatus::complete);
                  expect (Sysex8Event (1_gr, SysexStatus::start,     0, std::span (buf)).status == SysexStatus::start);
                  expect (Sysex8Event (1_gr, SysexStatus::continue_, 0, std::span (buf)).status == SysexStatus::continue_);
                  expect (Sysex8Event (1_gr, SysexStatus::end,       0, std::span (buf)).status == SysexStatus::end);
              });

        test ("Sysex8EndType::valid → numBytes 0x1, all data zero",
              [&] ()
              {
                  Sysex8Event event (1_gr, 0x42, Sysex8EndType::valid);
                  expect (event.status == SysexStatus::end);
                  expect (event.streamId == 0x42);
                  expect (event.numBytes == 0x1);
                  for (int i = 0; i < 13; ++i)
                      expect (event[i] == 0);
              });

        test ("Sysex8EndType::unknown → numBytes 0xF",
              [&] ()
              {
                  Sysex8Event event (1_gr, 0x42, Sysex8EndType::unknown);
                  expect (event.status == SysexStatus::end);
                  expect (event.numBytes == 0xF);
              });

        test ("roundtrip via ValueTree",
              [&] ()
              {
                  const uint8_t buf[] = { 0xFF, 0x80, 0x42 };
                  Sysex8Event event (1_gr, SysexStatus::complete, 0xAB, std::span (buf));
                  juce::ValueTree valueTree = event;
                  expect (valueTree.isValid ());
                  UmpEvent umpEvent (valueTree);
                  Sysex8Event event2 (umpEvent);
                  expect (event2.userGroup == 1);
                  expect (event2.status == SysexStatus::complete);
                  expect (event2.streamId == 0xAB);
                  expect (event2.numBytes == 4);
                  expect (event2[0] == 0xFF);
                  expect (event2[1] == 0x80);
                  expect (event2[2] == 0x42);
              });

        test ("factory: ≤13 bytes → complete, correct numBytes",
              [&] ()
              {
                  const uint8_t buf[] = { 0x41, 0x10, 0x42 };
                  int count = 0;
                  Sysex8EventFactory factory ([&] (Sysex8Event e)
                  {
                      ++count;
                      expect (e.status == SysexStatus::complete);
                      expect (e.numBytes == 4); // 3 data + 1 stream ID
                      expect (e[0] == 0x41);
                      expect (e[2] == 0x42);
                  });
                  factory.createEvents (1_gr, 0x01, std::span (buf));
                  expect (count == 1);
              });

        test ("factory: 14-byte span → start + end",
              [&] ()
              {
                  const uint8_t buf[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
                  int count = 0;
                  Sysex8EventFactory factory ([&] (Sysex8Event e)
                  {
                      if (count == 0)
                      {
                          expect (e.status == SysexStatus::start);
                          expect (e.numBytes == 14);
                      }
                      else if (count == 1)
                      {
                          expect (e.status == SysexStatus::end);
                          expect (e.numBytes == 2); // 1 data byte + 1 stream ID
                          expect (e[0] == 14);
                      }
                      ++count;
                  });
                  factory.createEvents (1_gr, 0x01, std::span (buf));
                  expect (count == 2);
              });

        test ("factory: 27-byte span → start + continue + end",
              [&] ()
              {
                  const uint8_t buf[] = {
                      1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13,
                      14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
                      27
                  };
                  int count = 0;
                  Sysex8EventFactory factory ([&] (Sysex8Event e)
                  {
                      if (count == 0)
                      {
                          expect (e.status == SysexStatus::start);
                          expect (e.numBytes == 14);
                      }
                      else if (count == 1)
                      {
                          expect (e.status == SysexStatus::continue_);
                          expect (e.numBytes == 14);
                      }
                      else if (count == 2)
                      {
                          expect (e.status == SysexStatus::end);
                          expect (e.numBytes == 2); // 1 data byte + 1 stream ID
                          expect (e[0] == 27);
                      }
                      ++count;
                  });
                  factory.createEvents (1_gr, 0x01, std::span (buf));
                  expect (count == 3);
              });

        test ("factory: null handler → no crash",
              [&] ()
              {
                  const uint8_t buf[] = { 1, 2, 3 };
                  Sysex8EventFactory factory (nullptr);
                  factory.createEvents (1_gr, 0x01, std::span (buf));
                  expect (true);
              });
    }
};

static Test_Sysex8 testSysex8;

class Test_MixedDataSet : public TestSuite
{
public:
    Test_MixedDataSet ()
    : TestSuite ("MixedDataSet", "!!! category !!!")
    {
    }

    void runTest () override
    {
        using namespace midi_literals;

        test ("header construction",
              [&] ()
              {
                  MixedDataSetHeaderEvent e (2_gr, 3, 100, 4, 1, 0x0041, 0x0010, 0x01, 0x02);
                  expect (e.userGroup    == 2);
                  expect (e.mdsId        == 3);
                  expect (e.numValidBytes == 100);
                  expect (e.numChunks    == 4);
                  expect (e.chunkNumber  == 1);
                  expect (e.manufacturerId == 0x0041);
                  expect (e.deviceId      == 0x0010);
                  expect (e.subId1        == 0x01);
                  expect (e.subId2        == 0x02);
                  expect (e.status        == SysexStatus::mdsHeader);
              });

        test ("header ValueTree roundtrip",
              [&] ()
              {
                  MixedDataSetHeaderEvent e (1_gr, 5, 28, 2, 1, 0x1234, 0x5678, 0xAB, 0xCD);
                  juce::ValueTree vt = e;
                  expect (vt.isValid ());
                  UmpEvent ump (vt);
                  MixedDataSetHeaderEvent e2 (ump);
                  expect (e2.userGroup     == 1);
                  expect (e2.mdsId         == 5);
                  expect (e2.numValidBytes == 28);
                  expect (e2.numChunks     == 2);
                  expect (e2.chunkNumber   == 1);
                  expect (e2.manufacturerId == 0x1234);
                  expect (e2.deviceId      == 0x5678);
                  expect (e2.subId1        == 0xAB);
                  expect (e2.subId2        == 0xCD);
              });

        test ("payload span construction",
              [&] ()
              {
                  const uint8_t buf[] = { 0x01, 0x02, 0x03, 0x04 };
                  MixedDataSetPayloadEvent e (1_gr, 7, std::span (buf));
                  expect (e.userGroup == 1);
                  expect (e.mdsId    == 7);
                  expect (e.status   == SysexStatus::mdsPayload);
                  expect (e[0] == 0x01);
                  expect (e[1] == 0x02);
                  expect (e[2] == 0x03);
                  expect (e[3] == 0x04);
                  expect (e[4] == 0);
              });

        test ("payload empty span → all data zero",
              [&] ()
              {
                  MixedDataSetPayloadEvent e (1_gr, 1, std::span<const uint8_t> {});
                  for (int i = 0; i < 14; ++i)
                      expect (e[i] == 0);
              });

        test ("payload 14-byte span fills all slots",
              [&] ()
              {
                  const uint8_t buf[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
                  MixedDataSetPayloadEvent e (1_gr, 1, std::span (buf));
                  for (int i = 0; i < 14; ++i)
                      expect (e[i] == i + 1);
              });

        test ("payload full 8-bit data preserved",
              [&] ()
              {
                  const uint8_t buf[] = { 0xFF, 0x80, 0xC0 };
                  MixedDataSetPayloadEvent e (1_gr, 1, std::span (buf));
                  expect (e[0] == 0xFF);
                  expect (e[1] == 0x80);
                  expect (e[2] == 0xC0);
              });

        test ("factory: null handlers → no crash",
              [&] ()
              {
                  const uint8_t buf[] = { 1, 2, 3 };
                  MixedDataSetFactory factory (nullptr, nullptr);
                  factory.createEvents (1_gr, 1, 0x0041, 0x10, 0x01, 0x02, std::span (buf));
                  expect (true);
              });

        test ("factory: ≤14 bytes → 1 header + 1 payload",
              [&] ()
              {
                  const uint8_t buf[] = { 0x41, 0x10, 0x42 };
                  int headers = 0, payloads = 0;
                  MixedDataSetFactory factory (
                      [&] (MixedDataSetHeaderEvent h)
                      {
                          ++headers;
                          expect (h.numValidBytes == 3);
                          expect (h.numChunks     == 1);
                          expect (h.chunkNumber   == 1);
                          expect (h.manufacturerId == 0x0041);
                      },
                      [&] (MixedDataSetPayloadEvent p)
                      {
                          ++payloads;
                          expect (p[0] == 0x41);
                          expect (p[1] == 0x10);
                          expect (p[2] == 0x42);
                      });
                  factory.createEvents (1_gr, 2, 0x0041, 0x10, 0x01, 0x02, std::span (buf));
                  expect (headers  == 1);
                  expect (payloads == 1);
              });

        test ("factory: 15-byte span → 1 header + 2 payloads",
              [&] ()
              {
                  const uint8_t buf[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
                  int headers = 0, payloads = 0;
                  MixedDataSetFactory factory (
                      [&] (MixedDataSetHeaderEvent h)
                      {
                          ++headers;
                          expect (h.numValidBytes == 15);
                          expect (h.numChunks     == 2);
                      },
                      [&] (MixedDataSetPayloadEvent p)
                      {
                          if (payloads == 0)
                          {
                              expect (p[0] == 1);
                              expect (p[13] == 14);
                          }
                          else
                          {
                              expect (p[0] == 15);
                              expect (p[1] == 0);
                          }
                          ++payloads;
                      });
                  factory.createEvents (1_gr, 1, 0, 0, 0, 0, std::span (buf));
                  expect (headers  == 1);
                  expect (payloads == 2);
              });

        test ("factory: 28-byte span → 1 header + 2 full payloads",
              [&] ()
              {
                  uint8_t buf[28];
                  for (int i = 0; i < 28; ++i) buf[i] = (uint8_t) (i + 1);
                  int headers = 0, payloads = 0;
                  MixedDataSetFactory factory (
                      [&] (MixedDataSetHeaderEvent h)
                      {
                          ++headers;
                          expect (h.numValidBytes == 28);
                          expect (h.numChunks     == 2);
                      },
                      [&] (MixedDataSetPayloadEvent p)
                      {
                          if (payloads == 0)
                              expect (p[0] == 1);
                          else
                              expect (p[0] == 15);
                          ++payloads;
                      });
                  factory.createEvents (1_gr, 1, 0, 0, 0, 0, std::span (buf));
                  expect (headers  == 1);
                  expect (payloads == 2);
              });
    }
};

static Test_MixedDataSet testMixedDataSet;

class Test_MfrId : public TestSuite
{
public:
    Test_MfrId ()
    : TestSuite ("MfrId", "!!! category !!!")
    {
    }

    void runTest () override
    {
        auto roundtrip = [&] (MidiByte id1, MidiByte id2, MidiByte id3)
        {
            const int mds = MfrIdToMdsFormat (id1, id2, id3);
            const auto [r1, r2, r3] = MdsFormatToMfrId (mds);
            expect (r1 == id1);
            expect (r2 == id2);
            expect (r3 == id3);
        };

        test ("1-byte IDs roundtrip",
              [&] ()
              {
                  roundtrip (0x01, 0, 0);   // Roland
                  roundtrip (0x07, 0, 0);   // Kurzweil
                  roundtrip (0x41, 0, 0);   // Roland (extended range)
                  roundtrip (0x7F, 0, 0);   // max 1-byte value
              });

        test ("3-byte IDs roundtrip",
              [&] ()
              {
                  roundtrip (0x00, 0x00, 0x01);  // Sequential / Dave Smith
                  roundtrip (0x00, 0x01, 0x0B);  // Korg
                  roundtrip (0x00, 0x20, 0x29);  // Focusrite
                  roundtrip (0x00, 0x7F, 0x7F);  // max 3-byte values
              });

        test ("1-byte: high bit of mds word clear",
              [&] ()
              {
                  const int mds = MfrIdToMdsFormat (0x41);
                  expect ((mds & 0x8000) == 0);
                  expect (mds == 0x41);
              });

        test ("3-byte: high bit of mds word set",
              [&] ()
              {
                  const int mds = MfrIdToMdsFormat (0x00, 0x20, 0x29);
                  expect ((mds & 0x8000) != 0);
              });
    }
};

static Test_MfrId testMfrId;
