
#include <juce_core/juce_core.h>

class Test_MdsMessage : public TestSuite
{
public:
    Test_MdsMessage ()
    : TestSuite ("MdsMessage", "sysex")
    {
    }

    void runTest () override
    {
        beginTest ("MdsMessage");

        test ("construction from all fields and buffer",
              [this] ()
              {
                  Buffer::Ptr buf { new Buffer () };
                  buf->append (0x41);
                  buf->append (0x10);
                  buf->append (0x42);

                  MdsMessage msg { MidiNibble { 2 }, 5, 0x0041, 0x0010, 0x01, 0x02, 3, buf };
                  expectEquals (static_cast<int> (msg.group),          2);
                  expectEquals (static_cast<int> (msg.mdsId),          5);
                  expectEquals (static_cast<int> (msg.manufacturerId), 0x0041);
                  expectEquals (static_cast<int> (msg.deviceId),       0x0010);
                  expectEquals (static_cast<int> (msg.subId1),         0x01);
                  expectEquals (static_cast<int> (msg.subId2),         0x02);
                  expectEquals (static_cast<int> (msg.numChunks),      3);
                  expect (msg.data.get () != nullptr);
                  expectEquals (static_cast<int> (msg.data.get ()->size ()), 3);
              });

        test ("type identifier is MsgMds",
              [this] ()
              {
                  Buffer::Ptr buf { new Buffer () };
                  MdsMessage msg { MidiNibble { 0 }, 0, 0, 0, 0, 0, 1, buf };
                  juce::ValueTree vt { msg };
                  expect (vt.hasType (MdsMessage::type));
              });

        test ("round-trip via Event constructor",
              [this] ()
              {
                  Buffer::Ptr buf { new Buffer () };
                  buf->append (0xDE);
                  buf->append (0xAD);

                  MdsMessage original { MidiNibble { 3 }, 7, 0x1234, 0x5678, 0xAB, 0xCD, 2, buf };
                  original.timestamp = 1.5;

                  Event base { original };
                  MdsMessage recovered { base };

                  expectEquals (static_cast<int> (recovered.group),          3);
                  expectEquals (static_cast<int> (recovered.mdsId),          7);
                  expectEquals (static_cast<int> (recovered.manufacturerId), 0x1234);
                  expectEquals (static_cast<int> (recovered.deviceId),       0x5678);
                  expectEquals (static_cast<int> (recovered.subId1),         0xAB);
                  expectEquals (static_cast<int> (recovered.subId2),         0xCD);
                  expectEquals (static_cast<int> (recovered.numChunks),      2);
                  expectEquals (static_cast<double> (recovered.timestamp),   1.5);
                  expect (recovered.data.get () != nullptr);
                  expectEquals (static_cast<int> (recovered.data.get ()->size ()), 2);
              });
    }
};

static Test_MdsMessage testMdsMessage;
