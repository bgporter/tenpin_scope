
#include <juce_core/juce_core.h>

class Test_CiProfile : public TestSuite
{
public:
    Test_CiProfile ()
    : TestSuite ("CiProfile", "ci")
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

    void checkCommonCiHeader (const Buffer& buf, int expectedType)
    {
        expectEquals (static_cast<int> (buf[0]), 0x7E, "buf[0] must be 0x7E, not 0xF0");
        expectEquals (static_cast<int> (buf[2]), 0x0D, "MIDI-CI sub-ID");
        expectEquals (static_cast<int> (buf[3]), expectedType, "message type");
        expect (static_cast<int> (buf[4]) >= messageFormatMin, "format must be >= 1");
        expect (noF0inBuf (buf), "Buffer must not contain 0xF0");
        expect (noF7inBuf (buf), "Buffer must not contain 0xF7");
    }

    ProfileId stdProfile () const { return { 0x7E, 0x00, 0x21, 0x01, 0x01 }; }
    ProfileId mfrProfile () const { return { 0x41, 0x10, 0x42, 0x01, 0x00 }; }

    void runTest () override
    {
        beginTest ("CiProfile");

        const MidiGroup group   { 1 };
        const int       srcMuid { 0x0654321 };
        const int       dstMuid { 0x01234567 };

        // ------------------------------------------------------------------
        test ("CiProfileInquiry: header only, correct type",
              [&, this] ()
              {
                  CiProfileInquiry inquiry { group, srcMuid, dstMuid };
                  auto msg = inquiry.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  expect (buf != nullptr);
                  checkCommonCiHeader (*buf, CiType::profileInquiry);
                  // Header-only: 13 bytes (no payload beyond common header)
                  expectEquals (static_cast<int> (buf->size ()), 13);
              });

        test ("CiProfileInquiry: round-trip via Event",
              [&, this] ()
              {
                  CiProfileInquiry original { group, srcMuid, dstMuid };
                  auto msg = original.toSysex7Message (MidiNibble { group }, 1);
                  CiProfileInquiry parsed { msg };
                  Event asEvent { parsed };
                  CiProfileInquiry recovered { asEvent };
                  expectEquals (recovered.sourceMuid.get (), srcMuid);
                  expectEquals (recovered.destMuid.get (),   dstMuid);
              });

        // ------------------------------------------------------------------
        test ("CiProfileInquiryReply: count fields are LSB-first",
              [&, this] ()
              {
                  CiProfileInquiryReply reply { group, srcMuid, dstMuid };
                  reply.addEnabledProfile  (stdProfile ());
                  reply.addEnabledProfile  (mfrProfile ());
                  reply.addDisabledProfile (stdProfile ());
                  auto msg = reply.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  expect (buf != nullptr);
                  checkCommonCiHeader (*buf, CiType::profileInquiryReply);
                  // cep at buf[13..14], LSB first
                  const int cep = wordFromBuf (*buf, 13);
                  expectEquals (cep, 2, "enabled profile count");
                  expectEquals (static_cast<int> ((*buf)[13]), 2, "cep LSB = 2");
                  expectEquals (static_cast<int> ((*buf)[14]), 0, "cep MSB = 0");
                  // cdp after cep + profiles
                  const size_t cdpOffset = 15 + 2 * 5;
                  const int cdp = wordFromBuf (*buf, cdpOffset);
                  expectEquals (cdp, 1, "disabled profile count");
              });

        test ("CiProfileInquiryReply: round-trip profiles",
              [&, this] ()
              {
                  CiProfileInquiryReply original { group, srcMuid, dstMuid };
                  original.addEnabledProfile  (stdProfile ());
                  original.addEnabledProfile  (mfrProfile ());
                  original.addDisabledProfile ({ 0x7E, 0x00, 0x24, 0x01, 0x01 });
                  auto msg = original.toSysex7Message (MidiNibble { group }, 1);
                  CiProfileInquiryReply recovered { msg };

                  expectEquals (recovered.enabledProfileCount (),  2);
                  expectEquals (recovered.disabledProfileCount (), 1);
                  expect (recovered.enabledProfileAt (0) == stdProfile ());
                  expect (recovered.enabledProfileAt (1) == mfrProfile ());
                  expectEquals (static_cast<int> (recovered.disabledProfileAt (0).byte3), 0x24);
              });

        test ("CiProfileInquiryReply: empty lists",
              [&, this] ()
              {
                  CiProfileInquiryReply reply { group, srcMuid, dstMuid };
                  auto msg = reply.toSysex7Message (MidiNibble { group }, 1);
                  CiProfileInquiryReply recovered { msg };
                  expectEquals (recovered.enabledProfileCount (),  0);
                  expectEquals (recovered.disabledProfileCount (), 0);
              });

        // ------------------------------------------------------------------
        test ("CiProfileSetOn: header correct, channelsRequested in v2",
              [&, this] ()
              {
                  CiProfileSetOn setOn { group, srcMuid, dstMuid, stdProfile (), 4 };
                  auto v1 = setOn.toSysex7Message (MidiNibble { group }, 1);
                  auto v2 = setOn.toSysex7Message (MidiNibble { group }, 2);
                  checkCommonCiHeader (*v1.data.get (), CiType::profileSetOn);
                  expect (v1.data.get ()->size () == 18, "v1: no channelsRequested");
                  expect (v2.data.get ()->size () == 20, "v2: channelsRequested added");

                  CiProfileSetOn r2 { v2 };
                  expectEquals (r2.channelsRequested.get (), 4);
                  expect (r2.profileId () == stdProfile ());
              });

        test ("CiProfileSetOn: round-trip profile bytes",
              [&, this] ()
              {
                  CiProfileSetOn original { group, srcMuid, dstMuid, mfrProfile (), 0 };
                  auto msg = original.toSysex7Message (MidiNibble { group }, 1);
                  CiProfileSetOn recovered { msg };
                  expect (recovered.profileId () == mfrProfile ());
                  expectEquals (recovered.sourceMuid.get (), srcMuid);
              });

        // ------------------------------------------------------------------
        test ("CiProfileSetOff: round-trip",
              [&, this] ()
              {
                  CiProfileSetOff original { group, srcMuid, dstMuid, stdProfile () };
                  auto msg = original.toSysex7Message (MidiNibble { group }, 1);
                  checkCommonCiHeader (*msg.data.get (), CiType::profileSetOff);
                  CiProfileSetOff recovered { msg };
                  expect (recovered.profileId () == stdProfile ());
                  expectEquals (recovered.sourceMuid.get (), srcMuid);
              });

        // ------------------------------------------------------------------
        test ("CiProfileEnabled/Disabled: channelsEnabled/Disabled in v2",
              [&, this] ()
              {
                  CiProfileEnabled enabled  { group, srcMuid, stdProfile (), 3 };
                  CiProfileDisabled disabled { group, srcMuid, stdProfile (), 3 };

                  auto ev1 = enabled.toSysex7Message  (MidiNibble { group }, 1);
                  auto ev2 = enabled.toSysex7Message  (MidiNibble { group }, 2);
                  auto dv1 = disabled.toSysex7Message (MidiNibble { group }, 1);
                  auto dv2 = disabled.toSysex7Message (MidiNibble { group }, 2);

                  checkCommonCiHeader (*ev1.data.get (), CiType::profileEnabled);
                  checkCommonCiHeader (*dv1.data.get (), CiType::profileDisabled);
                  expect (ev1.data.get ()->size () == 18, "Enabled v1: no channelsEnabled");
                  expect (ev2.data.get ()->size () == 20, "Enabled v2: channelsEnabled added");

                  CiProfileEnabled  re { ev2 };
                  CiProfileDisabled rd { dv2 };
                  expectEquals (re.channelsEnabled.get (),  3);
                  expectEquals (rd.channelsDisabled.get (), 3);
              });

        // ------------------------------------------------------------------
        test ("CiProfileAdded/Removed: round-trip",
              [&, this] ()
              {
                  CiProfileAdded   added   { group, srcMuid, stdProfile () };
                  CiProfileRemoved removed { group, srcMuid, mfrProfile () };

                  auto am = added.toSysex7Message   (MidiNibble { group }, 1);
                  auto rm = removed.toSysex7Message (MidiNibble { group }, 1);
                  checkCommonCiHeader (*am.data.get (), CiType::profileAdded);
                  checkCommonCiHeader (*rm.data.get (), CiType::profileRemoved);

                  CiProfileAdded   ra { am };
                  CiProfileRemoved rr { rm };
                  expect (ra.profileId () == stdProfile ());
                  expect (rr.profileId () == mfrProfile ());
              });

        // ------------------------------------------------------------------
        test ("CiProfileDetailsInquiry: round-trip",
              [&, this] ()
              {
                  CiProfileDetailsInquiry original { group, srcMuid, dstMuid,
                                                     stdProfile (), MidiByte { 0x01 } };
                  auto msg = original.toSysex7Message (MidiNibble { group }, 1);
                  checkCommonCiHeader (*msg.data.get (), CiType::profileDetailsInquiry);
                  CiProfileDetailsInquiry recovered { msg };
                  expect (recovered.profileId () == stdProfile ());
                  expectEquals (recovered.inquiryTarget.get (), 0x01);
                  expect (recovered.isRegisteredTarget ());
              });

        test ("CiProfileDetailsInquiryReply: always uses format 0x02, targetData length endianness",
              [&, this] ()
              {
                  Buffer::Ptr data = new Buffer ();
                  data->append (0x01);
                  data->append (0x02);
                  data->append (0x03);
                  CiProfileDetailsInquiryReply original { group, srcMuid, dstMuid,
                                                          stdProfile (), MidiByte { 0x01 }, data };
                  // targetFormat ignored — always serialized as format 2
                  auto msg = original.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  checkCommonCiHeader (*buf, CiType::profileDetailsInquiryReply);
                  expectEquals (static_cast<int> ((*buf)[4]), 0x02, "format byte always 0x02");

                  // targetDataLength at buf[19..20], LSB first: value = 3
                  const int dl = wordFromBuf (*buf, 19);
                  expectEquals (dl, 3, "targetData length LSB-first");

                  CiProfileDetailsInquiryReply recovered { msg };
                  expect (recovered.profileId () == stdProfile ());
                  expectEquals (recovered.inquiryTarget.get (), 0x01);
                  auto rd = recovered.targetData.get ();
                  expect (rd != nullptr);
                  expectEquals (static_cast<int> (rd->size ()), 3);
                  expectEquals (static_cast<int> ((*rd)[2]), 0x03);
              });

        // ------------------------------------------------------------------
        test ("CiProfileSpecificData: 4-byte data length (MidiLong) endianness",
              [&, this] ()
              {
                  Buffer::Ptr data = new Buffer ();
                  for (int i = 0; i < 6; ++i)
                      data->append (static_cast<uint8_t> (i + 1));

                  CiProfileSpecificData original { group, srcMuid, dstMuid, stdProfile (), data };
                  auto msg = original.toSysex7Message (MidiNibble { group }, 1);
                  auto buf = msg.data.get ();
                  checkCommonCiHeader (*buf, CiType::profileSpecificData);

                  // 4-byte data length at buf[18..21], MidiLong LSB first; value = 6
                  const int fromBuf = muidFromBuf (*buf, 18);
                  expectEquals (fromBuf, 6, "data length LSB-first (4 bytes)");
                  // LSB byte first
                  expectEquals (static_cast<int> ((*buf)[18]), 6, "data length LSB = 6");
                  expectEquals (static_cast<int> ((*buf)[19]), 0, "data length byte2 = 0");

                  CiProfileSpecificData recovered { msg };
                  expect (recovered.profileId () == stdProfile ());
                  auto rd = recovered.profileData.get ();
                  expect (rd != nullptr);
                  expectEquals (static_cast<int> (rd->size ()), 6);
                  expectEquals (static_cast<int> ((*rd)[5]), 6);
              });

        test ("ProfileId: isStandardDefined and operator==",
              [&, this] ()
              {
                  expect (stdProfile ().isStandardDefined ());
                  expect (!mfrProfile ().isStandardDefined ());
                  expect (stdProfile () == stdProfile ());
                  expect (!(stdProfile () == mfrProfile ()));
              });
    }
};

static Test_CiProfile testCiProfile;
