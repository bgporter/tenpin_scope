
#include <juce_core/juce_core.h>

class Test_Sysex8Builder : public TestSuite
{
public:
    Test_Sysex8Builder ()
    : TestSuite ("Sysex8Builder", "sysex")
    {
    }

    int messageCount (EventList& list)
    {
        int count = 0;
        juce::ValueTree vt { list };
        for (const auto& child : vt)
            if (child.hasType (Sysex8Message::type))
                ++count;
        return count;
    }

    Sysex8Message nthMessage (EventList& list, int n)
    {
        juce::ValueTree vt { list };
        int idx = 0;
        for (const auto& child : vt)
        {
            if (child.hasType (Sysex8Message::type))
            {
                if (idx++ == n)
                    return Sysex8Message { child };
            }
        }
        jassertfalse;
        return Sysex8Message { MidiNibble { 0 }, 0, nullptr };
    }

    void runTest () override
    {
        beginTest ("Sysex8Builder");

        test ("single complete packet",
              [this] ()
              {
                  using namespace midi_literals;
                  EventList list;
                  EventList assembled;
                  Sysex8Builder builder { list, [&assembled] (Event& msg) { assembled.addMessage (msg); } };

                  const uint8_t bytes[] = { 0x41, 0x10, 0x42, 0x12 };
                  Sysex8Event e { 1_gr, SysexStatus::complete, 5, std::span (bytes) };
                  list.addEvent (e);

                  expectEquals (messageCount (assembled), 1);
                  Sysex8Message msg { nthMessage (assembled, 0) };
                  expectEquals (static_cast<int> (msg.group), 0); // 0-based nibble
                  expectEquals (static_cast<int> (msg.streamId), 5);
                  auto buf = msg.data.get ();
                  expect (buf != nullptr);
                  expectEquals (static_cast<int> (buf->size ()), 4);
                  expectEquals (static_cast<int> ((*buf)[0]), static_cast<int> (0x41));
                  expectEquals (static_cast<int> ((*buf)[3]), static_cast<int> (0x12));
              });

        test ("multi-packet start/continue/end",
              [this] ()
              {
                  using namespace midi_literals;
                  EventList list;
                  EventList assembled;
                  Sysex8Builder builder { list, [&assembled] (Event& msg) { assembled.addMessage (msg); } };

                  const uint8_t start[]    = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                                               0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D };
                  const uint8_t continue_[] = { 0x0E, 0x0F, 0x10, 0x11, 0x12, 0x13,
                                                0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A };
                  const uint8_t end[]      = { 0x1B, 0x1C };

                  Sysex8Event startEvent    { 1_gr, SysexStatus::start,    3, std::span (start) };
                  Sysex8Event continueEvent { 1_gr, SysexStatus::continue_, 3, std::span (continue_) };
                  Sysex8Event endEvent      { 1_gr, SysexStatus::end,       3, std::span (end) };
                  list.addEvent (startEvent);
                  list.addEvent (continueEvent);
                  list.addEvent (endEvent);

                  expectEquals (messageCount (assembled), 1);
                  auto buf = nthMessage (assembled, 0).data.get ();
                  expect (buf != nullptr);
                  expectEquals (static_cast<int> (buf->size ()), 28);
                  expectEquals (static_cast<int> ((*buf)[0]),  static_cast<int> (0x01));
                  expectEquals (static_cast<int> ((*buf)[27]), static_cast<int> (0x1C));
              });

        test ("non-sysex8 events are ignored",
              [this] ()
              {
                  EventList list;
                  EventList assembled;
                  Sysex8Builder builder { list, [&assembled] (Event& msg) { assembled.addMessage (msg); } };

                  UmpEvent other;
                  list.addEvent (other);

                  expectEquals (messageCount (assembled), 0);
              });

        test ("two simultaneous streams on same group",
              [this] ()
              {
                  using namespace midi_literals;
                  EventList list;
                  EventList assembled;
                  Sysex8Builder builder { list, [&assembled] (Event& msg) { assembled.addMessage (msg); } };

                  const uint8_t bytesA[] = { 0xAA, 0xBB };
                  const uint8_t bytesB[] = { 0xCC, 0xDD, 0xEE };

                  // Interleave packets from stream 1 and stream 2 on group 1
                  Sysex8Event startA    { 1_gr, SysexStatus::start,    1, std::span (bytesA) };
                  Sysex8Event startB    { 1_gr, SysexStatus::start,    2, std::span (bytesB) };
                  Sysex8Event endA      { 1_gr, SysexStatus::end,      1, std::span<const uint8_t> {} };
                  Sysex8Event endB      { 1_gr, SysexStatus::end,      2, std::span<const uint8_t> {} };

                  list.addEvent (startA);
                  list.addEvent (startB);
                  list.addEvent (endA);
                  list.addEvent (endB);

                  expectEquals (messageCount (assembled), 2);

                  // Verify each message carries its own bytes
                  bool foundA = false, foundB = false;
                  juce::ValueTree vt { assembled };
                  for (const auto& child : vt)
                  {
                      if (!child.hasType (Sysex8Message::type))
                          continue;
                      Sysex8Message msg { child };
                      const int sid = static_cast<int> (msg.streamId);
                      auto buf = msg.data.get ();
                      expect (buf != nullptr);
                      if (sid == 1)
                      {
                          foundA = true;
                          expectEquals (static_cast<int> (buf->size ()), 2);
                          expectEquals (static_cast<int> ((*buf)[0]), static_cast<int> (0xAA));
                      }
                      else if (sid == 2)
                      {
                          foundB = true;
                          expectEquals (static_cast<int> (buf->size ()), 3);
                          expectEquals (static_cast<int> ((*buf)[0]), static_cast<int> (0xCC));
                      }
                  }
                  expect (foundA);
                  expect (foundB);
              });

        test ("provenance copied from final packet",
              [this] ()
              {
                  using namespace midi_literals;
                  EventList list;
                  EventList assembled;
                  Sysex8Builder builder { list, [&assembled] (Event& msg) { assembled.addMessage (msg); } };

                  const uint8_t bytes[] = { 0x7E };
                  Sysex8Event e { 1_gr, SysexStatus::complete, 0, std::span (bytes) };
                  e.timestamp     = 3.14;
                  e.endpointIndex = 7;
                  e.endpointName  = "Port1";
                  e.isReceived    = true;
                  list.addEvent (e);

                  Sysex8Message msg { nthMessage (assembled, 0) };
                  expectEquals (static_cast<double> (msg.timestamp), 3.14);
                  expectEquals (static_cast<int> (msg.endpointIndex), 7);
                  expect (static_cast<juce::String> (msg.endpointName) == "Port1");
                  expect (static_cast<bool> (msg.isReceived));
              });
    }
};

static Test_Sysex8Builder testSysex8Builder;
