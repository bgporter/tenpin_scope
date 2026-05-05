
#include <juce_core/juce_core.h>

class Test_Stream : public TestSuite
{
public:
    Test_Stream ()
    : TestSuite ("Stream", "!!! category !!!")
    {
    }

    void runTest () override
    {
        test ("EndpointDiscoveryEvent: construction",
              [&] ()
              {
                  EndpointDiscoveryEvent e (1, 1, true, true, false, false, false);
                  expect (e.messageType == MessageTypes::stream);
                  expect (e.format.get () == StreamFormat::complete);
                  expect (e.status == StreamStatus::endpointDiscovery);
                  expect (e.umpVersionMajor == 1);
                  expect (e.umpVersionMinor == 1);
                  expect (e.requestEndpointInfo == true);
                  expect (e.requestDeviceIdentity == true);
                  expect (e.requestEndpointName == false);
                  expect (e.requestProductInstanceId == false);
                  expect (e.requestStreamConfig == false);
              });

        test ("EndpointDiscoveryEvent: all flags",
              [&] ()
              {
                  EndpointDiscoveryEvent e (1, 1, true, true, true, true, true);
                  expect (e.requestEndpointInfo == true);
                  expect (e.requestDeviceIdentity == true);
                  expect (e.requestEndpointName == true);
                  expect (e.requestProductInstanceId == true);
                  expect (e.requestStreamConfig == true);
              });

        test ("EndpointDiscoveryEvent: ValueTree roundtrip",
              [&] ()
              {
                  EndpointDiscoveryEvent src (1, 1, true, false, true, false, true);
                  juce::ValueTree vt = src;
                  expect (vt.isValid ());
                  UmpEvent ump (vt);
                  EndpointDiscoveryEvent e2 (ump);
                  expect (e2.messageType == MessageTypes::stream);
                  expect (e2.status == StreamStatus::endpointDiscovery);
                  expect (e2.umpVersionMajor == 1);
                  expect (e2.umpVersionMinor == 1);
                  expect (e2.requestEndpointInfo == true);
                  expect (e2.requestDeviceIdentity == false);
                  expect (e2.requestEndpointName == true);
                  expect (e2.requestProductInstanceId == false);
                  expect (e2.requestStreamConfig == true);
              });

        test ("EndpointInfoNotificationEvent: construction",
              [&] ()
              {
                  EndpointInfoNotificationEvent e (1, 1, true, 3, true, true, false, true);
                  expect (e.messageType == MessageTypes::stream);
                  expect (e.status == StreamStatus::endpointInfoNotification);
                  expect (e.umpVersionMajor == 1);
                  expect (e.umpVersionMinor == 1);
                  expect (e.staticFunctionBlocks == true);
                  expect (e.numFunctionBlocks == 3);
                  expect (e.midi2Protocol == true);
                  expect (e.midi1Protocol == true);
                  expect (e.rxJrTimestamp == false);
                  expect (e.txJrTimestamp == true);
              });

        test ("EndpointInfoNotificationEvent: ValueTree roundtrip",
              [&] ()
              {
                  EndpointInfoNotificationEvent src (1, 0, false, 5, true, false, true, false);
                  juce::ValueTree vt = src;
                  UmpEvent ump (vt);
                  EndpointInfoNotificationEvent e2 (ump);
                  expect (e2.staticFunctionBlocks == false);
                  expect (e2.numFunctionBlocks == 5);
                  expect (e2.midi2Protocol == true);
                  expect (e2.midi1Protocol == false);
                  expect (e2.rxJrTimestamp == true);
                  expect (e2.txJrTimestamp == false);
              });

        test ("DeviceIdentityNotificationEvent: construction",
              [&] ()
              {
                  DeviceIdentityNotificationEvent e (0x00, 0x21, 0x09,
                                                     MidiWord (5), MidiWord (42),
                                                     1, 2, 3, 4);
                  expect (e.messageType == MessageTypes::stream);
                  expect (e.status == StreamStatus::deviceIdentityNotification);
                  expect (e.mfrId1 == 0x00);
                  expect (e.mfrId2 == 0x21);
                  expect (e.mfrId3 == 0x09);
                  expect (e.deviceFamily == 5);
                  expect (e.modelNumber == 42);
                  expect (e.swRev1 == 1);
                  expect (e.swRev2 == 2);
                  expect (e.swRev3 == 3);
                  expect (e.swRev4 == 4);
              });

        test ("DeviceIdentityNotificationEvent: family/model LSB-MSB packing",
              [&] ()
              {
                  // 300 = 0b10_0101100 → LSB = 0x2C (44), MSB = 0x02
                  DeviceIdentityNotificationEvent e (0, 0, 0, MidiWord (300), MidiWord (300), 0, 0, 0, 0);
                  expect (e.deviceFamily == 300);
                  expect (e.modelNumber == 300);
              });

        test ("DeviceIdentityNotificationEvent: ValueTree roundtrip",
              [&] ()
              {
                  DeviceIdentityNotificationEvent src (0x00, 0x41, 0x13,
                                                       MidiWord (7), MidiWord (99),
                                                       5, 6, 7, 8);
                  juce::ValueTree vt = src;
                  UmpEvent ump (vt);
                  DeviceIdentityNotificationEvent e2 (ump);
                  expect (e2.mfrId1 == 0x00);
                  expect (e2.mfrId2 == 0x41);
                  expect (e2.mfrId3 == 0x13);
                  expect (e2.deviceFamily == 7);
                  expect (e2.modelNumber == 99);
                  expect (e2.swRev1 == 5);
                  expect (e2.swRev2 == 6);
                  expect (e2.swRev3 == 7);
                  expect (e2.swRev4 == 8);
              });

        test ("StreamConfigurationRequestEvent: MIDI 2.0 with JR timestamps",
              [&] ()
              {
                  StreamConfigurationRequestEvent e (2, true, true);
                  expect (e.messageType == MessageTypes::stream);
                  expect (e.status == StreamStatus::streamConfigRequest);
                  expect (e.protocol == 2);
                  expect (e.rxJrTimestamp == true);
                  expect (e.txJrTimestamp == true);
              });

        test ("StreamConfigurationRequestEvent: MIDI 1.0 without JR timestamps",
              [&] ()
              {
                  StreamConfigurationRequestEvent e (1, false, false);
                  expect (e.protocol == 1);
                  expect (e.rxJrTimestamp == false);
                  expect (e.txJrTimestamp == false);
              });

        test ("StreamConfigurationRequestEvent: ValueTree roundtrip",
              [&] ()
              {
                  StreamConfigurationRequestEvent src (2, true, false);
                  juce::ValueTree vt = src;
                  UmpEvent ump (vt);
                  StreamConfigurationRequestEvent e2 (ump);
                  expect (e2.status == StreamStatus::streamConfigRequest);
                  expect (e2.protocol == 2);
                  expect (e2.rxJrTimestamp == true);
                  expect (e2.txJrTimestamp == false);
              });

        test ("StreamConfigurationNotificationEvent: construction",
              [&] ()
              {
                  StreamConfigurationNotificationEvent e (2, false, true);
                  expect (e.messageType == MessageTypes::stream);
                  expect (e.status == StreamStatus::streamConfigNotification);
                  expect (e.protocol == 2);
                  expect (e.rxJrTimestamp == false);
                  expect (e.txJrTimestamp == true);
              });

        test ("FunctionBlockDiscoveryEvent: request both",
              [&] ()
              {
                  FunctionBlockDiscoveryEvent e (0, true, true);
                  expect (e.messageType == MessageTypes::stream);
                  expect (e.status == StreamStatus::functionBlockDiscovery);
                  expect (e.functionBlockNumber == 0);
                  expect (e.requestInfo == true);
                  expect (e.requestName == true);
              });

        test ("FunctionBlockDiscoveryEvent: all blocks, info only",
              [&] ()
              {
                  FunctionBlockDiscoveryEvent e (0xFF, true, false);
                  expect (e.functionBlockNumber == 0xFF);
                  expect (e.requestInfo == true);
                  expect (e.requestName == false);
              });

        test ("FunctionBlockInfoNotificationEvent: construction",
              [&] ()
              {
                  FunctionBlockInfoNotificationEvent e (true, 2, 1, 0, 3, 0, 4, 0, 16);
                  expect (e.messageType == MessageTypes::stream);
                  expect (e.status == StreamStatus::functionBlockInfoNotification);
                  expect (e.active == true);
                  expect (e.functionBlockNumber == 2);
                  expect (e.uiHint == 1);
                  expect (e.midi1 == 0);
                  expect (e.direction == 3);
                  expect (e.firstGroup == 0);
                  expect (e.numGroups == 4);
                  expect (e.numMidi1Channels == 0);
                  expect (e.maxSysex8Streams == 16);
              });

        test ("StartOfClipEvent: construction",
              [&] ()
              {
                  StartOfClipEvent e;
                  expect (e.messageType == MessageTypes::stream);
                  expect (e.status == StreamStatus::startOfClip);
                  expect (e.format.get () == StreamFormat::complete);
                  expect (e.eventName == juce::String ("Stream: Start of Clip"));
              });

        test ("EndOfClipEvent: construction",
              [&] ()
              {
                  EndOfClipEvent e;
                  expect (e.messageType == MessageTypes::stream);
                  expect (e.status == StreamStatus::endOfClip);
                  expect (e.eventName == juce::String ("Stream: End of Clip"));
              });

        test ("StreamTextEventFactory: complete packet for short name",
              [&] ()
              {
                  std::vector<StreamTextEvent> packets;
                  StreamTextEventFactory factory ([&] (StreamTextEvent e) { packets.push_back (e); });
                  factory.createEvents (StreamStatus::endpointNameNotification, 0, "MyDevice");
                  expect (packets.size () == 1u);
                  expect (packets[0].format.get () == StreamFormat::complete);
                  expect (packets[0].status == StreamStatus::endpointNameNotification);
                  expect (packets[0][0] == 'M');
                  expect (packets[0][1] == 'y');
              });

        test ("StreamTextEventFactory: multi-packet for long name",
              [&] ()
              {
                  std::vector<StreamTextEvent> packets;
                  StreamTextEventFactory factory ([&] (StreamTextEvent e) { packets.push_back (e); });
                  // 25 chars → ceil(25/12) = 3 packets
                  factory.createEvents (StreamStatus::endpointNameNotification, 0,
                                        "My Very Long Device Name!");
                  expect (packets.size () == 3u);
                  expect (packets[0].format.get () == StreamFormat::start);
                  expect (packets[1].format.get () == StreamFormat::continue_);
                  expect (packets[2].format.get () == StreamFormat::end);
              });

        test ("StreamTextEventFactory: function block name",
              [&] ()
              {
                  std::vector<StreamTextEvent> packets;
                  StreamTextEventFactory factory ([&] (StreamTextEvent e) { packets.push_back (e); });
                  factory.createEvents (StreamStatus::functionBlockNameNotification, 3, "Keys");
                  expect (packets.size () == 1u);
                  expect (packets[0].functionBlockNumber == 3);
                  expect (packets[0].status == StreamStatus::functionBlockNameNotification);
                  expect (packets[0][0] == 'K');
              });

        test ("StreamTextEvent: ValueTree roundtrip",
              [&] ()
              {
                  std::vector<StreamTextEvent> packets;
                  StreamTextEventFactory factory ([&] (StreamTextEvent e) { packets.push_back (e); });
                  factory.createEvents (StreamStatus::productInstanceId, 0, "SN12345");
                  const auto& src    = packets[0];
                  juce::ValueTree vt = src;
                  UmpEvent ump (vt);
                  StreamTextEvent e2 (ump);
                  expect (e2.format.get () == StreamFormat::complete);
                  for (int i = 0; i < 7; ++i)
                      expect (e2[i] == src[i]);
                  expect (e2[7] == 0);
              });
    }

private:
    // !!! test class member vars here...
};

static Test_Stream testStream;
