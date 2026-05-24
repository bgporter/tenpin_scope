
#include <juce_core/juce_core.h>

class Test_CiEndpointInfo : public TestSuite
{
public:
    Test_CiEndpointInfo ()
    : TestSuite ("CiEndpointInfo", "ci")
    {
    }

    bool noF0inBuf (const Buffer& buf) const
    {
        for (size_t i = 0; i < buf.size (); ++i)
            if (buf[i] == 0xF0) return false;
        return true;
    }

    bool noF7inBuf (const Buffer& buf) const
    {
        for (size_t i = 0; i < buf.size (); ++i)
            if (buf[i] == 0xF7) return false;
        return true;
    }

    int muidFromBuf (const Buffer& buf, size_t offset) const
    {
        return static_cast<int> (buf[offset])
             | (static_cast<int> (buf[offset + 1]) << 7)
             | (static_cast<int> (buf[offset + 2]) << 14)
             | (static_cast<int> (buf[offset + 3]) << 21);
    }

    int wordFromBuf (const Buffer& buf, size_t offset) const
    {
        return static_cast<int> (buf[offset]) | (static_cast<int> (buf[offset + 1]) << 7);
    }

    void checkCommonCiHeader (const Buffer& buf, int expectedDeviceId,
                               int expectedType, int expectedFormat)
    {
        expectEquals (static_cast<int> (buf[0]), 0x7E, "buf[0] must be 0x7E, not 0xF0");
        expectEquals (static_cast<int> (buf[1]), expectedDeviceId, "device ID");
        expectEquals (static_cast<int> (buf[2]), 0x0D, "MIDI-CI sub-ID");
        expectEquals (static_cast<int> (buf[3]), expectedType, "message type");
        expect (static_cast<int> (buf[4]) >= messageFormatMin, "format must be >= 1");
        expect (noF0inBuf (buf), "Buffer must not contain 0xF0");
        expect (noF7inBuf (buf), "Buffer must not contain 0xF7");
    }

    void runTest () override
    {
        beginTest ("CiEndpointInfo");

        const MidiGroup group   { 1 };
        const int       srcMuid { 0x0654321 };
        const int       dstMuid { 0x01234567 };

        // ------------------------------------------------------------------
        test ("CiEndpointInquiry: common header is correct",
              [&, this] ()
              {
                  CiEndpointInquiry inquiry { group, srcMuid, dstMuid,
                                              MidiByte { CiEndpointStatus::productInstanceId } };
                  auto msg = inquiry.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  expect (buf != nullptr);
                  checkCommonCiHeader (*buf, CiDeviceId::functionBlock, CiType::endpointInquiry, 1);
              });

        test ("CiEndpointInquiry: MUID endianness",
              [&, this] ()
              {
                  CiEndpointInquiry inquiry { group, srcMuid, dstMuid,
                                              MidiByte { CiEndpointStatus::productInstanceId } };
                  auto msg = inquiry.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  expectEquals (muidFromBuf (*buf, 5), srcMuid, "srcMuid at buf[5..8]");
                  expectEquals (muidFromBuf (*buf, 9), dstMuid, "dstMuid at buf[9..12]");
                  expectEquals (static_cast<int> ((*buf)[5]), srcMuid & 0x7F, "srcMuid LSB first");
                  expectEquals (static_cast<int> ((*buf)[9]), dstMuid & 0x7F, "dstMuid LSB first");
              });

        test ("CiEndpointInquiry: round-trip",
              [&, this] ()
              {
                  CiEndpointInquiry original { group, srcMuid, dstMuid,
                                               MidiByte { CiEndpointStatus::productInstanceId } };
                  auto msg = original.toSysex7Message (MidiNibble { group }, 1);
                  CiEndpointInquiry recovered { msg };
                  expectEquals (recovered.sourceMuid.get (), srcMuid);
                  expectEquals (recovered.destMuid.get (),   dstMuid);
                  expectEquals (recovered.status.get (),     CiEndpointStatus::productInstanceId);
              });

        test ("CiEndpointInquiry: round-trip via Event",
              [&, this] ()
              {
                  CiEndpointInquiry original { group, srcMuid, dstMuid,
                                               MidiByte { CiEndpointStatus::productInstanceId } };
                  auto msg = original.toSysex7Message (MidiNibble { group }, 1);
                  CiEndpointInquiry parsed { msg };
                  Event asEvent { parsed };
                  CiEndpointInquiry recovered { asEvent };
                  expectEquals (recovered.sourceMuid.get (), srcMuid);
                  expectEquals (recovered.status.get (),     CiEndpointStatus::productInstanceId);
              });

        // ------------------------------------------------------------------
        test ("CiEndpointReply: common header is correct",
              [&, this] ()
              {
                  CiEndpointReply reply { group, srcMuid, dstMuid,
                                          MidiByte { CiEndpointStatus::productInstanceId },
                                          "SN20250401" };
                  auto msg = reply.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  expect (buf != nullptr);
                  checkCommonCiHeader (*buf, CiDeviceId::functionBlock, CiType::endpointReply, 1);
              });

        test ("CiEndpointReply: product ID length field endianness",
              [&, this] ()
              {
                  const juce::String pid { "SN20250401" }; // 10 chars
                  CiEndpointReply reply { group, srcMuid, dstMuid,
                                          MidiByte { CiEndpointStatus::productInstanceId }, pid };
                  auto msg = reply.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  // length at buf[14..15], LSB first
                  const int len = wordFromBuf (*buf, 14);
                  expectEquals (len, 10, "product ID length LSB-first at buf[14..15]");
                  // LSB is first (10 < 128, so LSB=10, MSB=0)
                  expectEquals (static_cast<int> ((*buf)[14]), 10);
                  expectEquals (static_cast<int> ((*buf)[15]), 0);
              });

        test ("CiEndpointReply: round-trip",
              [&, this] ()
              {
                  const juce::String pid { "SN20250401-SYNTH" };
                  CiEndpointReply original { group, srcMuid, dstMuid,
                                             MidiByte { CiEndpointStatus::productInstanceId }, pid };
                  auto msg = original.toSysex7Message (MidiNibble { group }, 1);
                  CiEndpointReply recovered { msg };
                  expectEquals (recovered.sourceMuid.get (),        srcMuid);
                  expectEquals (recovered.destMuid.get (),          dstMuid);
                  expectEquals (recovered.status.get (),            CiEndpointStatus::productInstanceId);
                  expect (static_cast<juce::String> (recovered.productInstanceId) == pid);
              });

        // ------------------------------------------------------------------
        test ("CiInvalidateMuid: common header is correct",
              [&, this] ()
              {
                  CiInvalidateMuid msg { group, srcMuid, dstMuid };
                  auto sysex = msg.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = sysex.data.get ();
                  expect (buf != nullptr);
                  checkCommonCiHeader (*buf, CiDeviceId::functionBlock, CiType::invalidateMuid, 1);
              });

        test ("CiInvalidateMuid: destMuid is broadcast",
              [&, this] ()
              {
                  CiInvalidateMuid msg { group, srcMuid, dstMuid };
                  auto sysex = msg.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = sysex.data.get ();
                  expectEquals (muidFromBuf (*buf, 9), broadcastMuid, "destMuid must be broadcast");
              });

        test ("CiInvalidateMuid: targetMuid endianness",
              [&, this] ()
              {
                  CiInvalidateMuid msg { group, srcMuid, dstMuid };
                  auto sysex = msg.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = sysex.data.get ();
                  // targetMuid at buf[13..16], LSB first
                  const int fromBuf = muidFromBuf (*buf, 13);
                  expectEquals (fromBuf, dstMuid, "targetMuid LSB-first at buf[13..16]");
                  expectEquals (static_cast<int> ((*buf)[13]), dstMuid & 0x7F, "targetMuid LSB first");
              });

        test ("CiInvalidateMuid: round-trip",
              [&, this] ()
              {
                  CiInvalidateMuid original { group, srcMuid, dstMuid };
                  auto sysex = original.toSysex7Message (MidiNibble { group }, 1);
                  CiInvalidateMuid recovered { sysex };
                  expectEquals (recovered.sourceMuid.get (), srcMuid);
                  expectEquals (recovered.destMuid.get (),   broadcastMuid);
                  expectEquals (recovered.targetMuid.get (), dstMuid);
              });
    }
};

static Test_CiEndpointInfo testCiEndpointInfo;
