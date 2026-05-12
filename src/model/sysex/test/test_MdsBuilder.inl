
#include <juce_core/juce_core.h>

class Test_MdsBuilder : public TestSuite
{
public:
    Test_MdsBuilder ()
    : TestSuite ("MdsBuilder", "sysex")
    {
    }

    int messageCount (EventList& list)
    {
        int count = 0;
        juce::ValueTree vt { list };
        for (const auto& child : vt)
            if (child.hasType (MdsMessage::type))
                ++count;
        return count;
    }

    MdsMessage nthMessage (EventList& list, int n)
    {
        juce::ValueTree vt { list };
        int idx = 0;
        for (const auto& child : vt)
        {
            if (child.hasType (MdsMessage::type))
            {
                if (idx++ == n)
                    return MdsMessage { child };
            }
        }
        jassertfalse;
        return MdsMessage { MidiNibble { 0 }, 0, 0, 0, 0, 0, 0, nullptr };
    }

    void runTest () override
    {
        beginTest ("MdsBuilder");

        test ("single-chunk message",
              [this] ()
              {
                  EventList list;
                  EventList assembled;
                  MdsBuilder builder { list, [&assembled] (Event& msg) { assembled.addMessage (msg); } };

                  const uint8_t bytes[] = { 0x41, 0x10, 0x42 };
                  MixedDataSetHeaderEvent  hdr { 1, 1, 3, 1, 1, 0x0041, 0x0010, 0x01, 0x02 };
                  MixedDataSetPayloadEvent pld { 1, 1, std::span (bytes) };
                  list.addEvent (hdr);
                  list.addEvent (pld);

                  expectEquals (messageCount (assembled), 1);
                  MdsMessage msg { nthMessage (assembled, 0) };
                  expectEquals (static_cast<int> (msg.group),          0); // 0-based
                  expectEquals (static_cast<int> (msg.mdsId),          1);
                  expectEquals (static_cast<int> (msg.manufacturerId), 0x0041);
                  expectEquals (static_cast<int> (msg.deviceId),       0x0010);
                  expectEquals (static_cast<int> (msg.subId1),         0x01);
                  expectEquals (static_cast<int> (msg.subId2),         0x02);
                  expectEquals (static_cast<int> (msg.numChunks),      1);
                  auto buf = msg.data.get ();
                  expect (buf != nullptr);
                  expectEquals (static_cast<int> (buf->size ()), 3);
                  expectEquals (static_cast<int> ((*buf)[0]), static_cast<int> (0x41));
                  expectEquals (static_cast<int> ((*buf)[2]), static_cast<int> (0x42));
              });

        test ("multi-chunk message",
              [this] ()
              {
                  EventList list;
                  EventList assembled;
                  MdsBuilder builder { list, [&assembled] (Event& msg) { assembled.addMessage (msg); } };

                  // 30 bytes = 14 + 14 + 2 across 3 payload packets
                  const uint8_t p1[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                         0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E };
                  const uint8_t p2[] = { 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                                         0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C };
                  const uint8_t p3[] = { 0x1D, 0x1E };

                  MixedDataSetHeaderEvent  hdr  { 1, 2, 30, 3, 1, 0x0041, 0x0010, 0x01, 0x01 };
                  MixedDataSetPayloadEvent pld1 { 1, 2, std::span (p1) };
                  MixedDataSetPayloadEvent pld2 { 1, 2, std::span (p2) };
                  MixedDataSetPayloadEvent pld3 { 1, 2, std::span (p3) };
                  list.addEvent (hdr);
                  list.addEvent (pld1);
                  list.addEvent (pld2);
                  list.addEvent (pld3);

                  expectEquals (messageCount (assembled), 1);
                  auto buf = nthMessage (assembled, 0).data.get ();
                  expect (buf != nullptr);
                  expectEquals (static_cast<int> (buf->size ()), 30);
                  expectEquals (static_cast<int> ((*buf)[0]),  static_cast<int> (0x01));
                  expectEquals (static_cast<int> ((*buf)[29]), static_cast<int> (0x1E));
              });

        test ("non-MDS events are ignored",
              [this] ()
              {
                  EventList list;
                  EventList assembled;
                  MdsBuilder builder { list, [&assembled] (Event& msg) { assembled.addMessage (msg); } };

                  UmpEvent other;
                  list.addEvent (other);

                  expectEquals (messageCount (assembled), 0);
              });

        test ("orphaned payload is ignored",
              [this] ()
              {
                  EventList list;
                  EventList assembled;
                  MdsBuilder builder { list, [&assembled] (Event& msg) { assembled.addMessage (msg); } };

                  const uint8_t bytes[] = { 0xAA, 0xBB };
                  MixedDataSetPayloadEvent pld { 1, 3, std::span (bytes) };
                  list.addEvent (pld);

                  expectEquals (messageCount (assembled), 0);
              });

        test ("two simultaneous streams on same group",
              [this] ()
              {
                  EventList list;
                  EventList assembled;
                  MdsBuilder builder { list, [&assembled] (Event& msg) { assembled.addMessage (msg); } };

                  const uint8_t bytesA[] = { 0xAA, 0xBB };
                  const uint8_t bytesB[] = { 0xCC, 0xDD, 0xEE };

                  // Interleave: header A, header B, payload A, payload B
                  MixedDataSetHeaderEvent  hdrA  { 1, 1, 2, 1, 1, 0x0001, 0x0002, 0x03, 0x04 };
                  MixedDataSetHeaderEvent  hdrB  { 1, 2, 3, 1, 1, 0x0005, 0x0006, 0x07, 0x08 };
                  MixedDataSetPayloadEvent pldA  { 1, 1, std::span (bytesA) };
                  MixedDataSetPayloadEvent pldB  { 1, 2, std::span (bytesB) };
                  list.addEvent (hdrA);
                  list.addEvent (hdrB);
                  list.addEvent (pldA);
                  list.addEvent (pldB);

                  expectEquals (messageCount (assembled), 2);

                  bool foundA = false, foundB = false;
                  juce::ValueTree vt { assembled };
                  for (const auto& child : vt)
                  {
                      if (!child.hasType (MdsMessage::type))
                          continue;
                      MdsMessage msg { child };
                      const int id = static_cast<int> (msg.mdsId);
                      auto buf = msg.data.get ();
                      expect (buf != nullptr);
                      if (id == 1)
                      {
                          foundA = true;
                          expectEquals (static_cast<int> (buf->size ()), 2);
                          expectEquals (static_cast<int> ((*buf)[0]), static_cast<int> (0xAA));
                      }
                      else if (id == 2)
                      {
                          foundB = true;
                          expectEquals (static_cast<int> (buf->size ()), 3);
                          expectEquals (static_cast<int> ((*buf)[0]), static_cast<int> (0xCC));
                      }
                  }
                  expect (foundA);
                  expect (foundB);
              });

        test ("new header resets incomplete stream",
              [this] ()
              {
                  EventList list;
                  EventList assembled;
                  MdsBuilder builder { list, [&assembled] (Event& msg) { assembled.addMessage (msg); } };

                  const uint8_t bytes[] = { 0x11, 0x22 };

                  // First header starts a stream, second header (same group/mdsId) replaces it.
                  MixedDataSetHeaderEvent  hdr1 { 1, 4, 10, 2, 1, 0x0001, 0x0002, 0x03, 0x04 };
                  MixedDataSetHeaderEvent  hdr2 { 1, 4, 2,  1, 1, 0x00AA, 0x00BB, 0xCC, 0xDD };
                  MixedDataSetPayloadEvent pld  { 1, 4, std::span (bytes) };
                  list.addEvent (hdr1);
                  list.addEvent (hdr2);
                  list.addEvent (pld);

                  expectEquals (messageCount (assembled), 1);
                  MdsMessage msg { nthMessage (assembled, 0) };
                  expectEquals (static_cast<int> (msg.manufacturerId), 0x00AA);
                  expectEquals (static_cast<int> (msg.numChunks),      1);
                  auto buf = msg.data.get ();
                  expect (buf != nullptr);
                  expectEquals (static_cast<int> (buf->size ()), 2);
              });

        test ("provenance copied from final payload",
              [this] ()
              {
                  EventList list;
                  EventList assembled;
                  MdsBuilder builder { list, [&assembled] (Event& msg) { assembled.addMessage (msg); } };

                  const uint8_t bytes[] = { 0x7E };
                  MixedDataSetHeaderEvent  hdr { 1, 1, 1, 1, 1, 0x0041, 0x0010, 0x01, 0x01 };
                  MixedDataSetPayloadEvent pld { 1, 1, std::span (bytes) };
                  pld.timestamp     = 3.14;
                  pld.endpointIndex = 7;
                  pld.endpointName  = "Port1";
                  pld.isReceived    = true;
                  list.addEvent (hdr);
                  list.addEvent (pld);

                  MdsMessage msg { nthMessage (assembled, 0) };
                  expectEquals (static_cast<double> (msg.timestamp), 3.14);
                  expectEquals (static_cast<int> (msg.endpointIndex), 7);
                  expect (static_cast<juce::String> (msg.endpointName) == "Port1");
                  expect (static_cast<bool> (msg.isReceived));
              });
    }
};

static Test_MdsBuilder testMdsBuilder;
