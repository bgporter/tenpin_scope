
#include <juce_core/juce_core.h>

class Test_Sysex7Builder : public TestSuite
{
public:
    Test_Sysex7Builder ()
    : TestSuite ("Sysex7Builder", "sysex")
    {
    }

    // Count children of the EventList whose ValueTree type is MsgSysex7
    int messageCount (EventList& list)
    {
        int count = 0;
        juce::ValueTree vt { list };
        for (const auto& child : vt)
            if (child.hasType (Sysex7Message::type))
                ++count;
        return count;
    }

    // Return the nth Sysex7Message child from the list (0-based)
    Sysex7Message nthMessage (EventList& list, int n)
    {
        juce::ValueTree vt { list };
        int idx = 0;
        for (const auto& child : vt)
        {
            if (child.hasType (Sysex7Message::type))
            {
                if (idx++ == n)
                    return Sysex7Message { child };
            }
        }
        jassertfalse;
        return Sysex7Message { MidiNibble { 0 }, nullptr };
    }

    void runTest () override
    {
        beginTest ("Sysex7Builder");

        test ("single complete packet",
              [this] ()
              {
                  using namespace midi_literals;
                  EventList list;
                  Sysex7Builder builder { list };

                  const uint8_t bytes[] = { 0x41, 0x10, 0x42, 0x12 };
                  Sysex7Event e { 1_gr, SysexStatus::complete, std::span (bytes) };
                  list.addEvent (e);

                  expectEquals (messageCount (list), 1);
                  Sysex7Message msg { nthMessage (list, 0) };
                  expectEquals (static_cast<int> (msg.group), 0); // group is 0-based nibble
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
                  Sysex7Builder builder { list };

                  const uint8_t start[]    = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
                  const uint8_t continue_[] = { 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C };
                  const uint8_t end[]      = { 0x0D, 0x0E };

                  Sysex7Event startEvent    { 1_gr, SysexStatus::start,    std::span (start) };
                  Sysex7Event continueEvent { 1_gr, SysexStatus::continue_, std::span (continue_) };
                  Sysex7Event endEvent      { 1_gr, SysexStatus::end,       std::span (end) };
                  list.addEvent (startEvent);
                  list.addEvent (continueEvent);
                  list.addEvent (endEvent);

                  expectEquals (messageCount (list), 1);
                  auto buf = nthMessage (list, 0).data.get ();
                  expect (buf != nullptr);
                  expectEquals (static_cast<int> (buf->size ()), 14);
                  expectEquals (static_cast<int> ((*buf)[0]),  static_cast<int> (0x01));
                  expectEquals (static_cast<int> ((*buf)[13]), static_cast<int> (0x0E));
              });

        test ("non-sysex7 events are ignored",
              [this] ()
              {
                  EventList list;
                  Sysex7Builder builder { list };

                  UmpEvent other;
                  list.addEvent (other);

                  expectEquals (messageCount (list), 0);
              });

        test ("provenance copied from final packet",
              [this] ()
              {
                  using namespace midi_literals;
                  EventList list;
                  Sysex7Builder builder { list };

                  const uint8_t bytes[] = { 0x7E };
                  Sysex7Event e { 1_gr, SysexStatus::complete, std::span (bytes) };
                  // Manually set provenance fields on the underlying UmpEvent
                  e.timestamp     = 3.14;
                  e.endpointIndex = 7;
                  e.endpointName  = "Port1";
                  e.isReceived    = true;
                  list.addEvent (e);

                  Sysex7Message msg { nthMessage (list, 0) };
                  expectEquals (static_cast<double> (msg.timestamp), 3.14);
                  expectEquals (static_cast<int> (msg.endpointIndex), 7);
                  expect (static_cast<juce::String> (msg.endpointName) == "Port1");
                  expect (static_cast<bool> (msg.isReceived));
              });
    }
};

static Test_Sysex7Builder testSysex7Builder;
