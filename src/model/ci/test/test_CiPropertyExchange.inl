
#include <juce_core/juce_core.h>
#include "ciTestHelpers.inl"

class Test_CiPropertyExchange : public CiTestHelpers
{
public:
    Test_CiPropertyExchange ()
    : CiTestHelpers ("CiPropertyExchange", "ci")
    {
    }

    void runTest () override
    {
        beginTest ("CiPropertyExchange");

        const MidiGroup group   { 1 };
        const int       srcMuid { 0x0654321 };
        const int       dstMuid { 0x01234567 };

        // ------------------------------------------------------------------
        test ("CiPeCapabilitiesInquiry: header correct, device ID is 0x7F",
              [&, this] ()
              {
                  CiPeCapabilitiesInquiry inquiry { group, srcMuid, dstMuid, 3 };
                  auto msg = inquiry.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  expect (buf != nullptr);
                  checkCommonCiHeader (*buf, CiDeviceId::functionBlock, CiType::peCapabilitiesInquiry, 1);
              });

        test ("CiPeCapabilitiesInquiry: v1 vs v2 size, majorVersion/minorVersion conditional",
              [&, this] ()
              {
                  CiPeCapabilitiesInquiry inquiry { group, srcMuid, dstMuid, 3, 1, 2 };
                  auto v1 = inquiry.toSysex7Message (MidiNibble { group }, 1);
                  auto v2 = inquiry.toSysex7Message (MidiNibble { group }, 2);
                  // v1: 13-byte common header + 1 (simultaneousRequests) = 14
                  expectEquals (static_cast<int> (v1.data.get ()->size ()), 14, "v1: 14 bytes");
                  // v2: 14 + majorVersion + minorVersion = 16
                  expectEquals (static_cast<int> (v2.data.get ()->size ()), 16, "v2: 16 bytes");

                  CiPeCapabilitiesInquiry r2 { v2 };
                  expectEquals (r2.majorVersion.get (), 1);
                  expectEquals (r2.minorVersion.get (), 2);
              });

        test ("CiPeCapabilitiesInquiry: round-trip all fields",
              [&, this] ()
              {
                  CiPeCapabilitiesInquiry original { group, srcMuid, dstMuid, 5, 1, 1 };
                  auto msg = original.toSysex7Message (MidiNibble { group }, 2);
                  CiPeCapabilitiesInquiry recovered { msg };
                  expectEquals (recovered.sourceMuid.get (),           srcMuid);
                  expectEquals (recovered.destMuid.get (),             dstMuid);
                  expectEquals (recovered.simultaneousRequests.get (), 5);
                  expectEquals (recovered.majorVersion.get (),         1);
                  expectEquals (recovered.minorVersion.get (),         1);
              });

        test ("CiPeCapabilitiesReply: round-trip via Event",
              [&, this] ()
              {
                  CiPeCapabilitiesReply original { group, srcMuid, dstMuid, 2, 1, 0 };
                  auto msg = original.toSysex7Message (MidiNibble { group }, 2);
                  CiPeCapabilitiesReply parsed { msg };
                  Event asEvent { parsed };
                  CiPeCapabilitiesReply recovered { asEvent };
                  expectEquals (recovered.sourceMuid.get (),           srcMuid);
                  expectEquals (recovered.simultaneousRequests.get (), 2);
                  expectEquals (recovered.majorVersion.get (),         1);
              });

        // ------------------------------------------------------------------
        test ("CiPeGetPropertyDataInquiry: header correct",
              [&, this] ()
              {
                  auto hdr = makeHeader (R"({"resource":"X"})");
                  CiPeGetPropertyDataInquiry inquiry { group, srcMuid, dstMuid, 1, hdr };
                  auto msg = inquiry.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  expect (buf != nullptr);
                  checkCommonCiHeader (*buf, CiDeviceId::functionBlock, CiType::peGetPropertyDataInquiry, 1);
              });

        test ("CiPeGetPropertyDataInquiry: headerDataLength endianness, fixed chunks in wire",
              [&, this] ()
              {
                  const char* json = R"({"resource":"X"})"; // 16 bytes
                  auto hdr = makeHeader (json);
                  const int hdrLen = static_cast<int> (hdr->size ());
                  CiPeGetPropertyDataInquiry inquiry { group, srcMuid, dstMuid, 7, hdr };
                  auto msg = inquiry.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  // headerDataLength at buf[14..15], LSB first
                  const int wl = wordFromBuf (*buf, 14);
                  expectEquals (wl, hdrLen, "headerDataLength round-trips correctly");
                  expectEquals (static_cast<int> ((*buf)[14]), hdrLen & 0x7F, "headerDataLength LSB");
                  expectEquals (static_cast<int> ((*buf)[15]), (hdrLen >> 7) & 0x7F, "headerDataLength MSB");
                  // Inquiry must have fixed numberOfChunks=1, chunkNumber=1, propertyDataLength=0
                  const size_t afterHdr = 16 + static_cast<size_t> (hdrLen);
                  expectEquals (wordFromBuf (*buf, afterHdr),     1, "numberOfChunks == 1");
                  expectEquals (wordFromBuf (*buf, afterHdr + 2), 1, "chunkNumber == 1");
                  expectEquals (wordFromBuf (*buf, afterHdr + 4), 0, "propertyDataLength == 0");
              });

        test ("CiPeGetPropertyDataInquiry: round-trip",
              [&, this] ()
              {
                  auto hdr = makeHeader (R"({"resource":"DeviceInfo"})");
                  CiPeGetPropertyDataInquiry original { group, srcMuid, dstMuid, 3, hdr };
                  auto msg = original.toSysex7Message (MidiNibble { group }, 1);
                  CiPeGetPropertyDataInquiry recovered { msg };
                  expectEquals (recovered.sourceMuid.get (), srcMuid);
                  expectEquals (recovered.requestId.get (),  3);
                  auto rd = recovered.headerData.get ();
                  expect (rd != nullptr);
                  expectEquals (static_cast<int> (rd->size ()), static_cast<int> (hdr->size ()));
                  expectEquals (static_cast<int> ((*rd)[0]), static_cast<int> ('{'));
              });

        test ("CiPeGetPropertyDataReply: header correct",
              [&, this] ()
              {
                  auto hdr  = makeHeader (R"({"status":200})");
                  auto data = makeData ({ 0x01, 0x02, 0x03 });
                  CiPeGetPropertyDataReply reply { group, srcMuid, dstMuid, 3, hdr, 2, 1, data };
                  auto msg = reply.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  expect (buf != nullptr);
                  checkCommonCiHeader (*buf, CiDeviceId::functionBlock, CiType::peGetPropertyDataReply, 1);
              });

        test ("CiPeGetPropertyDataReply: chunk field endianness (numberOfChunks = 256)",
              [&, this] ()
              {
                  auto hdr  = makeHeader ("{}"); // 2 bytes
                  auto data = makeData ({ 0x41, 0x42, 0x43, 0x44, 0x45 }); // 5 bytes
                  // numberOfChunks = 256 → LSB=0 (256 & 0x7F), MSB=2 (256 >> 7)
                  CiPeGetPropertyDataReply reply { group, srcMuid, dstMuid, 1, hdr, 256, 1, data };
                  auto msg = reply.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  // headerData is 2 bytes; numberOfChunks starts at buf[18]
                  const size_t chunksOff = 16 + 2;
                  expectEquals (static_cast<int> ((*buf)[chunksOff]),     0, "numberOfChunks LSB = 0");
                  expectEquals (static_cast<int> ((*buf)[chunksOff + 1]), 2, "numberOfChunks MSB = 2");
                  // chunkNumber = 1 → LSB=1, MSB=0
                  expectEquals (static_cast<int> ((*buf)[chunksOff + 2]), 1, "chunkNumber LSB = 1");
                  expectEquals (static_cast<int> ((*buf)[chunksOff + 3]), 0, "chunkNumber MSB = 0");
                  // propertyDataLength = 5 → LSB=5, MSB=0
                  expectEquals (wordFromBuf (*buf, chunksOff + 4), 5, "propertyDataLength = 5");
              });

        test ("CiPeGetPropertyDataReply: round-trip",
              [&, this] ()
              {
                  auto hdr  = makeHeader (R"({"status":200})");
                  auto data = makeData ({ 0x01, 0x7E, 0x02 }); // 0x7E is allowed inside data
                  CiPeGetPropertyDataReply original { group, srcMuid, dstMuid, 5, hdr, 1, 1, data };
                  auto msg = original.toSysex7Message (MidiNibble { group }, 1);
                  CiPeGetPropertyDataReply recovered { msg };
                  expectEquals (recovered.sourceMuid.get (),     srcMuid);
                  expectEquals (recovered.requestId.get (),      5);
                  expectEquals (recovered.numberOfChunks.get (), 1);
                  expectEquals (recovered.chunkNumber.get (),    1);
                  auto rd = recovered.propertyData.get ();
                  expect (rd != nullptr);
                  expectEquals (static_cast<int> (rd->size ()), 3);
                  expectEquals (static_cast<int> ((*rd)[1]), 0x7E);
              });

        // ------------------------------------------------------------------
        test ("CiPeSetPropertyDataInquiry: header correct",
              [&, this] ()
              {
                  auto hdr  = makeHeader (R"({"resource":"X"})");
                  auto data = makeData ({ 0x01, 0x02 });
                  CiPeSetPropertyDataInquiry inquiry { group, srcMuid, dstMuid, 2, hdr, 1, 1, data };
                  auto msg = inquiry.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  expect (buf != nullptr);
                  checkCommonCiHeader (*buf, CiDeviceId::functionBlock, CiType::peSetPropertyDataInquiry, 1);
              });

        test ("CiPeSetPropertyDataInquiry: round-trip",
              [&, this] ()
              {
                  auto hdr  = makeHeader (R"({"resource":"Foo"})");
                  auto data = makeData ({ 0x10, 0x20, 0x30, 0x40 });
                  CiPeSetPropertyDataInquiry original { group, srcMuid, dstMuid, 9, hdr, 3, 2, data };
                  auto msg = original.toSysex7Message (MidiNibble { group }, 1);
                  CiPeSetPropertyDataInquiry recovered { msg };
                  expectEquals (recovered.sourceMuid.get (),     srcMuid);
                  expectEquals (recovered.requestId.get (),      9);
                  expectEquals (recovered.numberOfChunks.get (), 3);
                  expectEquals (recovered.chunkNumber.get (),    2);
                  auto rd = recovered.propertyData.get ();
                  expect (rd != nullptr);
                  expectEquals (static_cast<int> (rd->size ()), 4);
                  expectEquals (static_cast<int> ((*rd)[3]), 0x40);
              });

        test ("CiPeSetPropertyDataReply: header correct, fixed chunks in wire",
              [&, this] ()
              {
                  auto hdr = makeHeader (R"({"status":200})");
                  CiPeSetPropertyDataReply reply { group, srcMuid, dstMuid, 9, hdr };
                  auto msg = reply.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  expect (buf != nullptr);
                  checkCommonCiHeader (*buf, CiDeviceId::functionBlock, CiType::peSetPropertyDataReply, 1);
                  // Fixed: numberOfChunks=1, chunkNumber=1, propertyDataLength=0
                  const int hl = wordFromBuf (*buf, 14);
                  const size_t afterHdr = 16 + static_cast<size_t> (hl);
                  expectEquals (wordFromBuf (*buf, afterHdr),     1, "numberOfChunks == 1");
                  expectEquals (wordFromBuf (*buf, afterHdr + 2), 1, "chunkNumber == 1");
                  expectEquals (wordFromBuf (*buf, afterHdr + 4), 0, "propertyDataLength == 0");
              });

        test ("CiPeSetPropertyDataReply: round-trip",
              [&, this] ()
              {
                  auto hdr = makeHeader (R"({"status":200})");
                  CiPeSetPropertyDataReply original { group, srcMuid, dstMuid, 4, hdr };
                  auto msg = original.toSysex7Message (MidiNibble { group }, 1);
                  CiPeSetPropertyDataReply recovered { msg };
                  expectEquals (recovered.sourceMuid.get (), srcMuid);
                  expectEquals (recovered.requestId.get (),  4);
                  auto rd = recovered.headerData.get ();
                  expect (rd != nullptr);
                  expectEquals (static_cast<int> (rd->size ()), static_cast<int> (hdr->size ()));
              });

        // ------------------------------------------------------------------
        test ("CiPeSubscriptionInquiry: header correct",
              [&, this] ()
              {
                  auto hdr  = makeHeader (R"({"command":"start"})");
                  auto data = makeData ({});
                  CiPeSubscriptionInquiry inquiry { group, srcMuid, dstMuid, 1, hdr, 1, 1, data };
                  auto msg = inquiry.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  expect (buf != nullptr);
                  checkCommonCiHeader (*buf, CiDeviceId::functionBlock, CiType::peSubscriptionInquiry, 1);
              });

        test ("CiPeSubscriptionInquiry: round-trip",
              [&, this] ()
              {
                  auto hdr  = makeHeader (R"({"command":"start","resource":"X"})");
                  auto data = makeData ({ 0x01, 0x02, 0x03 });
                  CiPeSubscriptionInquiry original { group, srcMuid, dstMuid, 6, hdr, 1, 1, data };
                  auto msg = original.toSysex7Message (MidiNibble { group }, 1);
                  CiPeSubscriptionInquiry recovered { msg };
                  expectEquals (recovered.sourceMuid.get (),     srcMuid);
                  expectEquals (recovered.requestId.get (),      6);
                  expectEquals (recovered.numberOfChunks.get (), 1);
                  expectEquals (recovered.chunkNumber.get (),    1);
                  auto rd = recovered.propertyData.get ();
                  expect (rd != nullptr);
                  expectEquals (static_cast<int> (rd->size ()), 3);
              });

        test ("CiPeSubscriptionReply: round-trip",
              [&, this] ()
              {
                  auto hdr  = makeHeader (R"({"status":200})");
                  auto data = makeData ({ 0x10, 0x11 });
                  CiPeSubscriptionReply original { group, srcMuid, dstMuid, 6, hdr, 1, 1, data };
                  auto msg = original.toSysex7Message (MidiNibble { group }, 1);
                  CiPeSubscriptionReply recovered { msg };
                  expectEquals (recovered.sourceMuid.get (), srcMuid);
                  expectEquals (recovered.requestId.get (),  6);
                  auto rd = recovered.propertyData.get ();
                  expect (rd != nullptr);
                  expectEquals (static_cast<int> ((*rd)[0]), 0x10);
              });

        test ("CiPeSubscriptionReply: round-trip via Event",
              [&, this] ()
              {
                  auto hdr  = makeHeader (R"({"status":200})");
                  auto data = makeData ({ 0x7E, 0x01 }); // 0x7E allowed inside data
                  CiPeSubscriptionReply original { group, srcMuid, dstMuid, 2, hdr, 1, 1, data };
                  auto msg = original.toSysex7Message (MidiNibble { group }, 1);
                  CiPeSubscriptionReply parsed { msg };
                  Event asEvent { parsed };
                  CiPeSubscriptionReply recovered { asEvent };
                  expectEquals (recovered.sourceMuid.get (), srcMuid);
                  expectEquals (recovered.requestId.get (),  2);
              });

        // ------------------------------------------------------------------
        test ("CiPeNotify: header correct, type byte is 0x3F",
              [&, this] ()
              {
                  auto hdr  = makeHeader (R"({"status":144})");
                  auto data = makeData ({ 0x01 });
                  CiPeNotify notify { group, srcMuid, dstMuid, 1, hdr, 1, 1, data };
                  auto msg = notify.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  expect (buf != nullptr);
                  checkCommonCiHeader (*buf, CiDeviceId::functionBlock, CiType::peNotify, 1);
                  expectEquals (static_cast<int> ((*buf)[3]), 0x3F, "Notify type byte is 0x3F");
              });

        test ("CiPeNotify: round-trip",
              [&, this] ()
              {
                  auto hdr  = makeHeader (R"({"status":144})");
                  auto data = makeData ({ 0x01, 0x02, 0x03, 0x04, 0x05 });
                  CiPeNotify original { group, srcMuid, dstMuid, 7, hdr, 1, 1, data };
                  auto msg = original.toSysex7Message (MidiNibble { group }, 1);
                  CiPeNotify recovered { msg };
                  expectEquals (recovered.sourceMuid.get (),     srcMuid);
                  expectEquals (recovered.requestId.get (),      7);
                  expectEquals (recovered.numberOfChunks.get (), 1);
                  expectEquals (recovered.chunkNumber.get (),    1);
                  auto rd = recovered.propertyData.get ();
                  expect (rd != nullptr);
                  expectEquals (static_cast<int> (rd->size ()), 5);
                  expectEquals (static_cast<int> ((*rd)[4]), 5);
              });
    }
};

static Test_CiPropertyExchange testCiPropertyExchange;
