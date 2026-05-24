
#include <juce_core/juce_core.h>
#include "ciTestHelpers.inl"

class Test_CiAck : public CiTestHelpers
{
public:
    Test_CiAck ()
    : CiTestHelpers ("CiAck", "ci")
    {
    }

    std::array<uint8_t, 5> makeDetails (uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint8_t e)
    {
        return { a, b, c, d, e };
    }

    void runTest () override
    {
        beginTest ("CiAck");

        const MidiGroup group   { 1 };
        const int       srcMuid { 0x0654321 };
        const int       dstMuid { 0x01234567 };
        const auto details = makeDetails (0x01, 0x02, 0x03, 0x04, 0x05);

        // ------------------------------------------------------------------
        test ("CiAck: common header is correct",
              [&, this] ()
              {
                  CiAck msg { group, srcMuid, dstMuid,
                              MidiByte { CiType::discoveryInquiry },
                              MidiByte { CiAckStatus::ack },
                              MidiByte { 0 },
                              details, "OK" };
                  auto sysex = msg.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = sysex.data.get ();
                  expect (buf != nullptr);
                  checkCommonCiHeader (*buf, CiDeviceId::functionBlock, CiType::ack, 1);
              });

        test ("CiAck: MUID endianness",
              [&, this] ()
              {
                  CiAck msg { group, srcMuid, dstMuid,
                              MidiByte { CiType::discoveryInquiry },
                              MidiByte { CiAckStatus::ack },
                              MidiByte { 0 },
                              details, "" };
                  auto sysex = msg.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = sysex.data.get ();
                  expectEquals (muidFromBuf (*buf, 5), srcMuid, "srcMuid at buf[5..8]");
                  expectEquals (muidFromBuf (*buf, 9), dstMuid, "dstMuid at buf[9..12]");
                  expectEquals (static_cast<int> ((*buf)[5]), srcMuid & 0x7F, "srcMuid LSB first");
              });

        test ("CiAck: message text length field endianness",
              [&, this] ()
              {
                  const juce::String text { "Hello" }; // 5 chars
                  CiAck msg { group, srcMuid, dstMuid,
                              MidiByte { CiType::discoveryInquiry },
                              MidiByte { CiAckStatus::ack },
                              MidiByte { 0 },
                              details, text };
                  auto sysex = msg.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = sysex.data.get ();
                  // message text length at buf[21..22], LSB first
                  const int len = wordFromBuf (*buf, 21);
                  expectEquals (len, 5, "text length LSB-first at buf[21..22]");
                  expectEquals (static_cast<int> ((*buf)[21]), 5, "text length LSB = 5");
                  expectEquals (static_cast<int> ((*buf)[22]), 0, "text length MSB = 0");
              });

        test ("CiAck: round-trip all fields",
              [&, this] ()
              {
                  const juce::String text { "Discovery acknowledged" };
                  CiAck original { group, srcMuid, dstMuid,
                                   MidiByte { CiType::discoveryInquiry },
                                   MidiByte { CiAckStatus::ack },
                                   MidiByte { 7 },
                                   details, text };
                  auto sysex = original.toSysex7Message (MidiNibble { group }, 1);
                  CiAck recovered { sysex };

                  expectEquals (recovered.sourceMuid.get (),    srcMuid);
                  expectEquals (recovered.destMuid.get (),      dstMuid);
                  expectEquals (recovered.originalSubId.get (), CiType::discoveryInquiry);
                  expectEquals (recovered.statusCode.get (),    CiAckStatus::ack);
                  expectEquals (recovered.statusData.get (),    7);
                  expectEquals (recovered.detail0.get (),    1);
                  expectEquals (recovered.detail1.get (),    2);
                  expectEquals (recovered.detail4.get (),    5);
                  expect (static_cast<juce::String> (recovered.messageText) == text);
              });

        test ("CiAck: round-trip via Event",
              [&, this] ()
              {
                  CiAck original { group, srcMuid, dstMuid,
                                   MidiByte { CiType::discoveryInquiry },
                                   MidiByte { CiAckStatus::ack },
                                   MidiByte { 0 },
                                   details, "test" };
                  auto sysex = original.toSysex7Message (MidiNibble { group }, 1);
                  CiAck parsed { sysex };
                  Event asEvent { parsed };
                  CiAck recovered { asEvent };
                  expectEquals (recovered.sourceMuid.get (), srcMuid);
                  expect (static_cast<juce::String> (recovered.messageText) == "test");
              });

        // ------------------------------------------------------------------
        test ("CiNak: common header is correct (type is 0x7F, not 0x7D)",
              [&, this] ()
              {
                  CiNak msg { group, srcMuid, dstMuid,
                              MidiByte { CiType::discoveryInquiry },
                              MidiByte { CiNakStatus::messageNotSupported },
                              MidiByte { 0 },
                              details, "not supported" };
                  auto sysex = msg.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = sysex.data.get ();
                  expect (buf != nullptr);
                  checkCommonCiHeader (*buf, CiDeviceId::functionBlock, CiType::nak, 1);
                  // Confirm it's NAK (0x7F), not ACK (0x7D)
                  expectEquals (static_cast<int> ((*buf)[3]), 0x7F, "NAK type byte");
              });

        test ("CiNak: round-trip all fields",
              [&, this] ()
              {
                  const juce::String text { "Discovery not supported on this device" };
                  CiNak original { group, srcMuid, dstMuid,
                                   MidiByte { CiType::discoveryInquiry },
                                   MidiByte { CiNakStatus::messageNotSupported },
                                   MidiByte { 3 },
                                   details, text };
                  auto sysex = original.toSysex7Message (MidiNibble { group }, 1);
                  CiNak recovered { sysex };

                  expectEquals (recovered.sourceMuid.get (),    srcMuid);
                  expectEquals (recovered.destMuid.get (),      dstMuid);
                  expectEquals (recovered.originalSubId.get (), CiType::discoveryInquiry);
                  expectEquals (recovered.statusCode.get (),    CiNakStatus::messageNotSupported);
                  expectEquals (recovered.statusData.get (),    3);
                  expectEquals (recovered.detail0.get (),    1);
                  expectEquals (recovered.detail4.get (),    5);
                  expect (static_cast<juce::String> (recovered.messageText) == text);
              });

        test ("CiNak and CiAck have different type bytes",
              [&, this] ()
              {
                  expectEquals (CiType::nak, 0x7F);
                  expectEquals (CiType::ack, 0x7D);
                  // Confirm they're distinct
                  expect (CiType::nak != CiType::ack);
              });
    }
};

static Test_CiAck testCiAck;
