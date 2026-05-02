

#include <juce_core/juce_core.h>

class Test_SystemCommon : public TestSuite
{
public:
    Test_SystemCommon ()
    : TestSuite ("SystemCommon", "!!! category !!!")
    {
    }

    void runTest () override
    {
        using namespace midi_literals;

        test ("MIDI Time Code - simple creation",
              [&] ()
              {
                  MidiTimeCodeEvent event (1_gr, MtcMessageType::minutesCountLsb, 11);
                  expect (event.userGroup == 1);
                  expect (event.group == 0);
                  expect (event.messageType == MessageTypes::systemCommon);
                  expect (event.status == UmpValues::SystemCommon::midiTimeCode);
                  expect (event.mtcType == MtcMessageType::minutesCountLsb);
                  expect (event.mtcData == 11);
              });

        test ("MIDI Time Code - roundtrip",
              [&] ()
              {
                  MidiTimeCodeEvent event (1_gr, MtcMessageType::secondsCountMsb, 7);
                  juce::ValueTree valueTree = event;
                  expect (valueTree.isValid ());
                  UmpEvent umpEvent (valueTree);
                  MidiTimeCodeEvent event2 (umpEvent);
                  expect (event2.userGroup == 1);
                  expect (event2.group == 0);
                  expect (event2.status == UmpValues::SystemCommon::midiTimeCode);
                  expect (event2.mtcType == MtcMessageType::secondsCountMsb);
                  expect (event2.mtcData == 7);
              });

        test ("Song Position Pointer",
              [&] ()
              {
                  SongPositionPointerEvent event (1_gr, 1024);
                  expect (event.userGroup == 1);
                  expect (event.status == UmpValues::SystemCommon::songPositionPointer);
                  expect (event.value == 1024);
                  expect (event.lsb == (1024 & 0x7F));
                  expect (event.msb == (1024 >> 7));
              });

        test ("Song Position Pointer - roundtrip",
              [&] ()
              {
                  SongPositionPointerEvent event (2_gr, 8192);
                  juce::ValueTree valueTree = event;
                  UmpEvent umpEvent (valueTree);
                  SongPositionPointerEvent event2 (umpEvent);
                  expect (event2.userGroup == 2);
                  expect (event2.value == 8192);
              });

        test ("Song Select",
              [&] ()
              {
                  SongSelectEvent event (2_gr, 5);
                  expect (event.userGroup == 2);
                  expect (event.status == UmpValues::SystemCommon::songSelect);
                  expect (event.song == 5);
              });

        test ("Tune Request",
              [&] ()
              {
                  TuneRequestEvent event (1_gr);
                  expect (event.userGroup == 1);
                  expect (event.messageType == MessageTypes::systemCommon);
                  expect (event.status == UmpValues::SystemCommon::tuneRequest);
              });

        test ("Timing Clock",
              [&] ()
              {
                  TimingClockEvent event (1_gr);
                  expect (event.messageType == MessageTypes::systemCommon);
                  expect (event.status == UmpValues::SystemCommon::timingClock);
              });

        test ("Start",
              [&] ()
              {
                  StartEvent event (1_gr);
                  expect (event.status == UmpValues::SystemCommon::start);
              });

        test ("Continue",
              [&] ()
              {
                  ContinueEvent event (1_gr);
                  expect (event.status == UmpValues::SystemCommon::continue_);
              });

        test ("Stop",
              [&] ()
              {
                  StopEvent event (1_gr);
                  expect (event.status == UmpValues::SystemCommon::stop);
              });

        test ("Active Sensing",
              [&] ()
              {
                  ActiveSensingEvent event (1_gr);
                  expect (event.status == UmpValues::SystemCommon::activeSensing);
              });

        test ("System Reset",
              [&] ()
              {
                  SystemResetEvent event (1_gr);
                  expect (event.status == UmpValues::SystemCommon::systemReset);
              });
    }

private:
    // !!! test class member vars here...
};

static Test_SystemCommon testSystemCommon;
