
#include <juce_core/juce_core.h>

class Test_TextBuilder : public TestSuite
{
public:
    Test_TextBuilder ()
    : TestSuite ("TextBuilder", "sysex")
    {
    }

    int messageCount (EventList& list)
    {
        int count = 0;
        juce::ValueTree vt { list };
        for (const auto& child : vt)
            if (TextMessage::isTextMessage (Event { child.getType ().toString (), child }))
                ++count;
        return count;
    }

    TextMessage nthTextMessage (EventList& list, int n)
    {
        juce::ValueTree vt { list };
        int idx = 0;
        for (const auto& child : vt)
        {
            Event e { child.getType ().toString (), child };
            if (TextMessage::isTextMessage (e))
            {
                if (idx++ == n)
                    return TextMessage { child };
            }
        }
        jassertfalse;
        return TextMessage { TextMessage::typeProjectName.toString (), 0, 0, 0, 0, 0, {} };
    }

    void runTest () override
    {
        beginTest ("TextBuilder");

        test ("single-packet FlexData complete",
              [this] ()
              {
                  EventList list;
                  EventList assembled;
                  TextBuilder builder { list, [&assembled] (Event& msg) { assembled.addMessage (msg); } };

                  auto addText = [&list] (FlexDataTextEvent e) { list.addEvent (e); };
                  FlexDataTextEventFactory factory (addText);
                  factory.createEvents (1, FlexDataStatusBank::metadataText,
                                        static_cast<int> (MetadataTextStatus::compositionName),
                                        "Symphony No.5");

                  expectEquals (messageCount (assembled), 1);
                  TextMessage msg { nthTextMessage (assembled, 0) };
                  juce::ValueTree vt { msg };
                  expect (vt.hasType (TextMessage::typeCompositionName));
                  expectEquals (static_cast<juce::String> (msg.text), juce::String ("Symphony No.5"));
              });

        test ("multi-packet FlexData start/continue/end",
              [this] ()
              {
                  EventList list;
                  EventList assembled;
                  TextBuilder builder { list, [&assembled] (Event& msg) { assembled.addMessage (msg); } };

                  // 30-byte string → 3 packets (12 + 12 + 6 bytes)
                  const juce::String longText = "ABCDEFGHIJKLMNOPQRSTUVWXYZ1234";
                  auto addText = [&list] (FlexDataTextEvent e) { list.addEvent (e); };
                  FlexDataTextEventFactory factory (addText);
                  factory.createEvents (1, FlexDataStatusBank::performanceText,
                                        static_cast<int> (PerformanceTextStatus::lyrics),
                                        longText.toStdString ());

                  expectEquals (messageCount (assembled), 1);
                  TextMessage msg { nthTextMessage (assembled, 0) };
                  juce::ValueTree vt { msg };
                  expect (vt.hasType (TextMessage::typeLyrics));
                  expectEquals (static_cast<juce::String> (msg.text), longText);
              });

        test ("two simultaneous FlexData streams (different status)",
              [this] ()
              {
                  // Different status values → different keys → each assembles independently.
                  // Short texts (≤ 12 bytes) → single "complete" packet each — no interleaving needed.
                  EventList list;
                  EventList assembled;
                  TextBuilder builder { list, [&assembled] (Event& msg) { assembled.addMessage (msg); } };

                  auto addText = [&list] (FlexDataTextEvent e) { list.addEvent (e); };
                  FlexDataTextEventFactory factory (addText);
                  factory.createEvents (1, FlexDataStatusBank::metadataText,
                                        static_cast<int> (MetadataTextStatus::composerName),
                                        "J.S. Bach");
                  factory.createEvents (1, FlexDataStatusBank::metadataText,
                                        static_cast<int> (MetadataTextStatus::lyricistName),
                                        "Handel");

                  expectEquals (messageCount (assembled), 2);
              });

        test ("orphaned FlexData end packet is ignored",
              [this] ()
              {
                  // Send only the last (end) packet of a multi-packet sequence — no preceding start.
                  EventList list;
                  EventList assembled;
                  TextBuilder builder { list, [&assembled] (Event& msg) { assembled.addMessage (msg); } };

                  const std::string text = "AAAAAAAAAAAA1111111111112222"; // 28 bytes → 3 packets
                  const int totalPkts    = (static_cast<int> (text.size ()) + 11) / 12;
                  int pktIdx             = 0;
                  auto addOnlyLast = [&list, &pktIdx, totalPkts] (FlexDataTextEvent e)
                  {
                      if (++pktIdx == totalPkts)
                          list.addEvent (e);
                  };
                  FlexDataTextEventFactory factory (addOnlyLast);
                  factory.createEvents (1, FlexDataStatusBank::metadataText,
                                        static_cast<int> (MetadataTextStatus::composerName), text);

                  expectEquals (messageCount (assembled), 0);
              });

        test ("new FlexData start resets incomplete stream",
              [this] ()
              {
                  // Send only the start packet of a multi-packet sequence, then send a complete
                  // packet for the same key — the complete resets the in-progress stream and
                  // immediately assembles a new message with just the second text.
                  EventList list;
                  EventList assembled;
                  TextBuilder builder { list, [&assembled] (Event& msg) { assembled.addMessage (msg); } };

                  // "FirstComposer!" is 14 bytes → start + end packets; capture only start
                  int pktIdx = 0;
                  auto addOnlyStart = [&list, &pktIdx] (FlexDataTextEvent e)
                  {
                      if (++pktIdx == 1)
                          list.addEvent (e);
                  };
                  FlexDataTextEventFactory f1 (addOnlyStart);
                  f1.createEvents (1, FlexDataStatusBank::metadataText,
                                   static_cast<int> (MetadataTextStatus::composerName),
                                   "FirstComposer!");

                  // "Second" is 6 bytes → single "complete" packet; resets and assembles
                  auto addAll = [&list] (FlexDataTextEvent e) { list.addEvent (e); };
                  FlexDataTextEventFactory f2 (addAll);
                  f2.createEvents (1, FlexDataStatusBank::metadataText,
                                   static_cast<int> (MetadataTextStatus::composerName),
                                   "Second");

                  expectEquals (messageCount (assembled), 1);
                  TextMessage msg { nthTextMessage (assembled, 0) };
                  expectEquals (static_cast<juce::String> (msg.text), juce::String ("Second"));
              });

        test ("StreamText complete → MsgEndpointName",
              [this] ()
              {
                  EventList list;
                  EventList assembled;
                  TextBuilder builder { list, [&assembled] (Event& msg) { assembled.addMessage (msg); } };

                  auto add = [&list] (StreamTextEvent e) { list.addEvent (e); };
                  StreamTextEventFactory factory (add);
                  factory.createEvents (StreamStatus::endpointNameNotification, 0, "My Device");

                  expectEquals (messageCount (assembled), 1);
                  TextMessage msg { nthTextMessage (assembled, 0) };
                  juce::ValueTree vt { msg };
                  expect (vt.hasType (TextMessage::typeEndpointName));
                  expectEquals (static_cast<juce::String> (msg.text), juce::String ("My Device"));
              });

        test ("multi-packet StreamText",
              [this] ()
              {
                  EventList list;
                  EventList assembled;
                  TextBuilder builder { list, [&assembled] (Event& msg) { assembled.addMessage (msg); } };

                  auto add = [&list] (StreamTextEvent e) { list.addEvent (e); };
                  StreamTextEventFactory factory (add);
                  // 25-byte product instance ID → 3 packets
                  factory.createEvents (StreamStatus::productInstanceId, 0, "ABCDEFGHIJKLMNOPQRSTUVWXY");

                  expectEquals (messageCount (assembled), 1);
                  TextMessage msg { nthTextMessage (assembled, 0) };
                  juce::ValueTree vt { msg };
                  expect (vt.hasType (TextMessage::typeProductInstanceId));
                  expectEquals (static_cast<juce::String> (msg.text),
                                juce::String ("ABCDEFGHIJKLMNOPQRSTUVWXY"));
              });

        test ("two simultaneous function block names",
              [this] ()
              {
                  // Different functionBlockNumbers → different keys → assemble independently.
                  // No interleaving needed: add all of fb0, then all of fb1.
                  EventList list;
                  EventList assembled;
                  TextBuilder builder { list, [&assembled] (Event& msg) { assembled.addMessage (msg); } };

                  auto add = [&list] (StreamTextEvent e) { list.addEvent (e); };
                  StreamTextEventFactory factory (add);
                  factory.createEvents (StreamStatus::functionBlockNameNotification, 0, "Keys");
                  factory.createEvents (StreamStatus::functionBlockNameNotification, 1, "Drums");

                  expectEquals (messageCount (assembled), 2);

                  bool foundFb0 = false, foundFb1 = false;
                  juce::ValueTree vt { assembled };
                  for (const auto& child : vt)
                  {
                      Event e { child.getType ().toString (), child };
                      if (!TextMessage::isTextMessage (e))
                          continue;
                      TextMessage msg { child };
                      if (static_cast<int> (msg.functionBlockNumber) == 0)
                      {
                          foundFb0 = true;
                          expectEquals (static_cast<juce::String> (msg.text), juce::String ("Keys"));
                      }
                      else if (static_cast<int> (msg.functionBlockNumber) == 1)
                      {
                          foundFb1 = true;
                          expectEquals (static_cast<juce::String> (msg.text), juce::String ("Drums"));
                      }
                  }
                  expect (foundFb0);
                  expect (foundFb1);
              });

        test ("provenance copied from last packet",
              [this] ()
              {
                  EventList list;
                  EventList assembled;
                  TextBuilder builder { list, [&assembled] (Event& msg) { assembled.addMessage (msg); } };

                  const std::string text = "AAAAAAAAAAAA1111111111112222"; // 28 bytes → 3 packets
                  const int totalPkts    = (static_cast<int> (text.size ()) + 11) / 12;
                  int pktIdx             = 0;

                  StreamTextEventFactory factory (
                      [&] (StreamTextEvent e)
                      {
                          ++pktIdx;
                          if (pktIdx == totalPkts)
                          {
                              e.timestamp     = 2.71;
                              e.endpointIndex = 3;
                              e.endpointName  = "Port2";
                              e.isReceived    = true;
                          }
                          list.addEvent (e);
                      });
                  factory.createEvents (StreamStatus::endpointNameNotification, 0, text);

                  TextMessage msg { nthTextMessage (assembled, 0) };
                  expectEquals (static_cast<double>       (msg.timestamp),     2.71);
                  expectEquals (static_cast<int>          (msg.endpointIndex), 3);
                  expect       (static_cast<juce::String> (msg.endpointName) == "Port2");
                  expect       (static_cast<bool>         (msg.isReceived));
              });
    }
};

static Test_TextBuilder testTextBuilder;
