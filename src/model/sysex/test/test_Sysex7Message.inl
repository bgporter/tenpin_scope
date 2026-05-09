
#include <juce_core/juce_core.h>

class Test_Sysex7Message : public TestSuite
{
public:
    Test_Sysex7Message ()
    : TestSuite ("Sysex7Message", "sysex")
    {
    }

    void runTest () override
    {
        beginTest ("Sysex7Message");

        test ("construction from group and buffer",
              [this] ()
              {
                  Buffer::Ptr buf { new Buffer () };
                  buf->append (0x41);
                  buf->append (0x10);
                  buf->append (0x42);

                  Sysex7Message msg { MidiNibble { 3 }, buf };
                  expectEquals (static_cast<int> (msg.group), 3);
                  expect (msg.data.get () != nullptr);
                  expectEquals (static_cast<int> (msg.data.get ()->size ()), 3);
              });

        test ("provenance fields",
              [this] ()
              {
                  Buffer::Ptr buf { new Buffer () };
                  Sysex7Message msg { MidiNibble { 0 }, buf };
                  msg.timestamp     = 1.5;
                  msg.endpointIndex = 2;
                  msg.endpointName  = "TestEndpoint";
                  msg.isReceived    = true;

                  expectEquals (static_cast<double> (msg.timestamp), 1.5);
                  expectEquals (static_cast<int> (msg.endpointIndex), 2);
                  expect (static_cast<juce::String> (msg.endpointName) == "TestEndpoint");
                  expect (static_cast<bool> (msg.isReceived));
              });

        test ("round-trip via Event constructor",
              [this] ()
              {
                  Buffer::Ptr buf { new Buffer () };
                  buf->append (0x7E);
                  buf->append (0x01);

                  Sysex7Message original { MidiNibble { 5 }, buf };
                  original.timestamp = 2.0;

                  // Re-wrap through the Event base
                  Event base { original };
                  Sysex7Message recovered { base };

                  expectEquals (static_cast<int> (recovered.group), 5);
                  expectEquals (static_cast<double> (recovered.timestamp), 2.0);
                  expect (recovered.data.get () != nullptr);
                  expectEquals (static_cast<int> (recovered.data.get ()->size ()), 2);
              });
    }
};

static Test_Sysex7Message testSysex7Message;
