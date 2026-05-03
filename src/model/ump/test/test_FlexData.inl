
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

        test ("bpmToTenNs: 120 BPM",
              [&] ()
              {
                  expect (bpmToTenNs (120.0) == 50'000'000u);
              });

        test ("bpmToTenNs: 60 BPM",
              [&] ()
              {
                  expect (bpmToTenNs (60.0) == 100'000'000u);
              });

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
                  expect (e.userGroup         == 2);
                  expect (e.format            == FlexDataFormat::complete);
                  expect (e.address           == FlexDataAddress::group);
                  expect (e.channel           == 0);
                  expect (e.statusBank        == 0);
                  expect (e.status            == 0);
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
                  expect (e2.userGroup           == 3);
                  expect (e2.format              == FlexDataFormat::complete);
                  expect (e2.address             == FlexDataAddress::group);
                  expect (e2.statusBank          == 0);
                  expect (e2.status              == 0);
                  expect (e2.tenNsPerQuarterNote == 50'000'000u);
              });

        test ("FlexDataEvent base: Word 0 fields parse correctly",
              [&] ()
              {
                  SetTempoEvent src (1_gr, 120.0);
                  juce::ValueTree vt = src;
                  UmpEvent ump (vt);
                  FlexDataEvent e (ump);
                  expect (e.userGroup  == 1);
                  expect (e.format     == FlexDataFormat::complete);
                  expect (e.address    == FlexDataAddress::group);
                  expect (e.channel    == 0);
                  expect (e.statusBank == 0);
                  expect (e.status     == 0);
              });
    }
};

static Test_FlexData testFlexData;
