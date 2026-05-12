
#include <juce_core/juce_core.h>

class Test_TextMessage : public TestSuite
{
public:
    Test_TextMessage ()
    : TestSuite ("TextMessage", "sysex")
    {
    }

    void runTest () override
    {
        beginTest ("TextMessage");

        test ("FlexData construction and field round-trip",
              [this] ()
              {
                  TextMessage msg { TextMessage::typeCompositionName.toString (),
                                    2, 5, 1, 1, 2, "Moonlight Sonata" };
                  expectEquals (static_cast<juce::String> (msg.text),       juce::String ("Moonlight Sonata"));
                  expectEquals (static_cast<int>          (msg.group),      2);
                  expectEquals (static_cast<int>          (msg.channel),    5);
                  expectEquals (static_cast<int>          (msg.address),    1);
                  expectEquals (static_cast<int>          (msg.statusBank), 1);
                  expectEquals (static_cast<int>          (msg.status),     2);
                  juce::ValueTree vt { msg };
                  expect (vt.hasType (TextMessage::typeCompositionName));
              });

        test ("StreamText construction and field round-trip",
              [this] ()
              {
                  TextMessage msg { TextMessage::typeFunctionBlockName.toString (),
                                    3, "Keys" };
                  expectEquals (static_cast<juce::String> (msg.text),                juce::String ("Keys"));
                  expectEquals (static_cast<int>          (msg.functionBlockNumber), 3);
                  juce::ValueTree vt { msg };
                  expect (vt.hasType (TextMessage::typeFunctionBlockName));
              });

        test ("isTextMessage, displayName, category helpers",
              [this] ()
              {
                  TextMessage flex { TextMessage::typeLyrics.toString (), 0, 0, 1, 2, 1, "la" };
                  expect    (TextMessage::isTextMessage (flex));
                  expect    (TextMessage::isFlexDataTextMessage (TextMessage::typeLyrics.toString ()));
                  expect    (!TextMessage::isStreamTextMessage (TextMessage::typeLyrics.toString ()));
                  expectEquals (TextMessage::displayName (TextMessage::typeLyrics.toString ()),
                                juce::String ("Lyrics"));

                  TextMessage stream { TextMessage::typeEndpointName.toString (), 0, "Dev" };
                  expect    (TextMessage::isTextMessage (stream));
                  expect    (TextMessage::isStreamTextMessage (TextMessage::typeEndpointName.toString ()));
                  expect    (!TextMessage::isFlexDataTextMessage (TextMessage::typeEndpointName.toString ()));
                  expectEquals (TextMessage::displayName (TextMessage::typeEndpointName.toString ()),
                                juce::String ("Endpoint Name"));

                  expectEquals (TextMessage::typeForFlexStatus (1, 2),
                                TextMessage::typeCompositionName.toString ());
                  expectEquals (TextMessage::typeForStreamStatus (0x03),
                                TextMessage::typeEndpointName.toString ());
                  expect (TextMessage::typeForFlexStatus (0, 99).isEmpty ());
              });

        test ("round-trip via Event constructor",
              [this] ()
              {
                  TextMessage original { TextMessage::typeArrangerName.toString (),
                                         1, 2, 1, 1, 7, "Ravel" };
                  original.timestamp = 9.99;

                  Event base { original };
                  TextMessage recovered { base };
                  expectEquals (static_cast<juce::String> (recovered.text),      juce::String ("Ravel"));
                  expectEquals (static_cast<int>          (recovered.group),     1);
                  expectEquals (static_cast<int>          (recovered.status),    7);
                  expectEquals (static_cast<double>       (recovered.timestamp), 9.99);
                  juce::ValueTree vt { recovered };
                  expect (vt.hasType (TextMessage::typeArrangerName));
              });
    }
};

static Test_TextMessage testTextMessage;
