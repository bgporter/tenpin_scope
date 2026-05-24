
#include <juce_core/juce_core.h>
#include "ciTestHelpers.inl"

class Test_CiDiscovery : public CiTestHelpers
{
public:
    Test_CiDiscovery ()
    : CiTestHelpers ("CiDiscovery", "ci")
    {
    }

    void runTest () override
    {
        beginTest ("CiDiscovery");

        const MidiGroup  group         { 1 };
        const int        srcMuid       { 0x0654321 };
        const int        dstMuid       { 0x01234567 };
        const ManufacturerId mfr       { MidiByte { 0x41 }, MidiByte { 0x00 }, MidiByte { 0x00 } };
        const DeviceFamily   family    { MidiByte { 0x42 }, MidiByte { 0x00 } };
        const DeviceFamilyModel model  { MidiByte { 0x12 }, MidiByte { 0x00 } };
        const int        categories    { CiCategory::profileConfiguration | CiCategory::propertyExchange };
        const MidiLong   maxSize       { 512 };

        // ------------------------------------------------------------------
        test ("CiDiscoveryInquiry: common header is correct",
              [&, this] ()
              {
                  CiDiscoveryInquiry inquiry { group, srcMuid, mfr, family, model,
                                               MidiByte { 1 }, MidiByte { 2 }, MidiByte { 0 }, MidiByte { 0 },
                                               categories, maxSize };
                  auto msg = inquiry.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  expect (buf != nullptr);
                  checkCommonCiHeader (*buf, CiDeviceId::functionBlock, CiType::discoveryInquiry, 1);
              });

        test ("CiDiscoveryInquiry: MUID endianness",
              [&, this] ()
              {
                  CiDiscoveryInquiry inquiry { group, srcMuid, mfr, family, model,
                                               MidiByte { 1 }, MidiByte { 2 }, MidiByte { 0 }, MidiByte { 0 },
                                               categories, maxSize };
                  auto msg = inquiry.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  // src MUID at [5..8], dst MUID (broadcast) at [9..12]
                  expectEquals (muidFromBuf (*buf, 5), srcMuid, "srcMuid LSB-first at buf[5..8]");
                  // LSB byte is first
                  expectEquals (static_cast<int> ((*buf)[5]), srcMuid & 0x7F, "srcMuid LSB at buf[5]");
              });

        test ("CiDiscoveryInquiry: maxSysexSize endianness (MidiLong, 4 bytes)",
              [&, this] ()
              {
                  CiDiscoveryInquiry inquiry { group, srcMuid, mfr, family, model,
                                               MidiByte { 1 }, MidiByte { 2 }, MidiByte { 0 }, MidiByte { 0 },
                                               categories, maxSize };
                  auto msg = inquiry.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  // maxSysexSize is at buf[25..28] (after 13-byte header + 12 bytes payload)
                  const int fromBuf = muidFromBuf (*buf, 25);
                  expectEquals (fromBuf, maxSize.get (), "maxSysexSize reconstructed from buf");
              });

        test ("CiDiscoveryInquiry: round-trip all fields",
              [&, this] ()
              {
                  CiDiscoveryInquiry original { group, srcMuid, mfr, family, model,
                                                MidiByte { 1 }, MidiByte { 2 }, MidiByte { 3 }, MidiByte { 4 },
                                                categories, maxSize };
                  auto msg = original.toSysex7Message (MidiNibble { group }, 1);
                  CiDiscoveryInquiry recovered { msg };

                  expectEquals (recovered.sourceMuid.get (),        srcMuid);
                  expectEquals (recovered.destMuid.get (),          broadcastMuid);
                  expectEquals (recovered.manufacturerByte0.get (), 0x41);
                  expectEquals (recovered.manufacturerByte1.get (), 0x00);
                  expectEquals (recovered.deviceFamilyLsb.get (),   0x42);
                  expectEquals (recovered.deviceFamilyMsb.get (),   0x00);
                  expectEquals (recovered.deviceFamilyModelLsb.get (), 0x12);
                  expectEquals (recovered.revisionByte0.get (),     1);
                  expectEquals (recovered.revisionByte1.get (),     2);
                  expectEquals (recovered.revisionByte2.get (),     3);
                  expectEquals (recovered.revisionByte3.get (),     4);
                  expectEquals (recovered.ciCategoriesSupported.get (), categories);
                  expectEquals (recovered.maxSysexSize.get (),      maxSize.get ());
                  expect (recovered.profileConfigSupported.get ());
                  expect (recovered.propertyExchangeSupported.get ());
                  expect (!recovered.protocolNegotiationSupported.get ());
              });

        test ("CiDiscoveryInquiry: format-2 outputPathId included only when requested",
              [&, this] ()
              {
                  CiDiscoveryInquiry inquiry { group, srcMuid, mfr, family, model,
                                               MidiByte { 1 }, MidiByte { 0 }, MidiByte { 0 }, MidiByte { 0 },
                                               categories, maxSize };
                  inquiry.outputPathId = 0x05;

                  auto v1 = inquiry.toSysex7Message (MidiNibble { group }, 1);
                  auto v2 = inquiry.toSysex7Message (MidiNibble { group }, 2);
                  expect (v1.data.get ()->size () == 29, "format 1: no outputPathId");
                  expect (v2.data.get ()->size () == 30, "format 2: outputPathId appended");

                  CiDiscoveryInquiry r2 { v2 };
                  expectEquals (r2.outputPathId.get (), 5);
              });

        // ------------------------------------------------------------------
        test ("CiDiscoveryReply: common header is correct",
              [&, this] ()
              {
                  CiDiscoveryReply reply { group, srcMuid, dstMuid, mfr, family, model,
                                           MidiByte { 1 }, MidiByte { 0 }, MidiByte { 0 }, MidiByte { 0 },
                                           categories, maxSize };
                  auto msg = reply.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  expect (buf != nullptr);
                  checkCommonCiHeader (*buf, CiDeviceId::functionBlock, CiType::discoveryReply, 1);
              });

        test ("CiDiscoveryReply: MUID endianness",
              [&, this] ()
              {
                  CiDiscoveryReply reply { group, srcMuid, dstMuid, mfr, family, model,
                                           MidiByte { 1 }, MidiByte { 0 }, MidiByte { 0 }, MidiByte { 0 },
                                           categories, maxSize };
                  auto msg = reply.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  expectEquals (muidFromBuf (*buf, 5),  srcMuid, "srcMuid at buf[5..8]");
                  expectEquals (muidFromBuf (*buf, 9),  dstMuid, "dstMuid at buf[9..12]");
                  // Explicitly verify LSB is first
                  expectEquals (static_cast<int> ((*buf)[5]), srcMuid & 0x7F, "srcMuid LSB first");
                  expectEquals (static_cast<int> ((*buf)[9]), dstMuid & 0x7F, "dstMuid LSB first");
              });

        test ("CiDiscoveryReply: round-trip all fields",
              [&, this] ()
              {
                  CiDiscoveryReply original { group, srcMuid, dstMuid, mfr, family, model,
                                              MidiByte { 1 }, MidiByte { 2 }, MidiByte { 3 }, MidiByte { 4 },
                                              categories, maxSize };
                  auto msg = original.toSysex7Message (MidiNibble { group }, 1);
                  CiDiscoveryReply recovered { msg };

                  expectEquals (recovered.sourceMuid.get (), srcMuid);
                  expectEquals (recovered.destMuid.get (),   dstMuid);
                  expectEquals (recovered.manufacturerByte0.get (), 0x41);
                  expectEquals (recovered.deviceFamilyLsb.get (),   0x42);
                  expectEquals (recovered.deviceFamilyModelLsb.get (), 0x12);
                  expectEquals (recovered.revisionByte0.get (), 1);
                  expectEquals (recovered.revisionByte3.get (), 4);
                  expectEquals (recovered.ciCategoriesSupported.get (), categories);
                  expectEquals (recovered.maxSysexSize.get (), maxSize.get ());
              });

        test ("CiDiscoveryReply: format-2 v2 fields included only when requested",
              [&, this] ()
              {
                  CiDiscoveryReply reply { group, srcMuid, dstMuid, mfr, family, model,
                                           MidiByte { 1 }, MidiByte { 0 }, MidiByte { 0 }, MidiByte { 0 },
                                           categories, maxSize };
                  reply.outputPathId = 0x03;
                  reply.functionBlock = 0x02;

                  auto v1 = reply.toSysex7Message (MidiNibble { group }, 1);
                  auto v2 = reply.toSysex7Message (MidiNibble { group }, 2);
                  expect (v1.data.get ()->size () == 29, "format 1: no v2 fields");
                  expect (v2.data.get ()->size () == 31, "format 2: outputPathId + functionBlock");

                  CiDiscoveryReply r2 { v2 };
                  expectEquals (r2.outputPathId.get (),  3);
                  expectEquals (r2.functionBlock.get (), 2);
              });

        test ("CiDiscoveryInquiry: round-trip via Event",
              [&, this] ()
              {
                  CiDiscoveryInquiry original { group, srcMuid, mfr, family, model,
                                                MidiByte { 1 }, MidiByte { 0 }, MidiByte { 0 }, MidiByte { 0 },
                                                categories, maxSize };
                  auto msg = original.toSysex7Message (MidiNibble { group }, 1);
                  CiDiscoveryInquiry parsed { msg };
                  Event asEvent { parsed };
                  CiDiscoveryInquiry recovered { asEvent };

                  expectEquals (recovered.sourceMuid.get (), srcMuid);
                  expectEquals (recovered.maxSysexSize.get (), maxSize.get ());
              });
    }
};

static Test_CiDiscovery testCiDiscovery;
