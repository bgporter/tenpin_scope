
#include <juce_core/juce_core.h>

class Test_Sysex8Message : public TestSuite
{
public:
    Test_Sysex8Message ()
    : TestSuite ("Sysex8Message", "sysex")
    {
    }

    void runTest () override
    {
        beginTest ("Sysex8Message");

        test ("construction from group, streamId, and buffer",
              [this] ()
              {
                  Buffer::Ptr buf { new Buffer () };
                  buf->append (0x41);
                  buf->append (0x10);
                  buf->append (0x42);

                  Sysex8Message msg { MidiNibble { 3 }, 7, buf };
                  expectEquals (static_cast<int> (msg.group), 3);
                  expectEquals (static_cast<int> (msg.streamId), 7);
                  expect (msg.data.get () != nullptr);
                  expectEquals (static_cast<int> (msg.data.get ()->size ()), 3);
              });

        test ("type identifier is MsgSysex8",
              [this] ()
              {
                  Buffer::Ptr buf { new Buffer () };
                  Sysex8Message msg { MidiNibble { 0 }, 0, buf };
                  juce::ValueTree vt { msg };
                  expect (vt.hasType (Sysex8Message::type));
              });

        test ("round-trip via Event constructor",
              [this] ()
              {
                  Buffer::Ptr buf { new Buffer () };
                  buf->append (0xDE);
                  buf->append (0xAD);

                  Sysex8Message original { MidiNibble { 5 }, 2, buf };
                  original.timestamp = 2.0;

                  Event base { original };
                  Sysex8Message recovered { base };

                  expectEquals (static_cast<int> (recovered.group), 5);
                  expectEquals (static_cast<int> (recovered.streamId), 2);
                  expectEquals (static_cast<double> (recovered.timestamp), 2.0);
                  expect (recovered.data.get () != nullptr);
                  expectEquals (static_cast<int> (recovered.data.get ()->size ()), 2);
              });
    }
};

static Test_Sysex8Message testSysex8Message;
