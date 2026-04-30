

#include <juce_core/juce_core.h>

class Test_ChannelVoice1 : public TestSuite
{
public:
    Test_ChannelVoice1 ()
    : TestSuite ("ChannelVoice1", "!!! category !!!")
    {
    }

    void runTest () override
    {
        using namespace midi_literals;

        test ("note off - simple creation",
              [&] ()
              {
                  Midi1NoteOffEvent event (1_gr, 2_ch, 60, 100);
                  expect (event.userGroup == 1);
                  expect (event.group == 0);
                  expect (event.status == UmpValues::ChannelVoice::noteOff);
                  expect (event.userChannel == 2);
                  expect (event.channel == 1);
                  expect (event.note == 60);
                  expect (event.velocity == 100);
              });

        test ("note on - simple creation",
              [&] ()
              {
                  Midi1NoteOnEvent event (1_gr, 2_ch, 64, 127);
                  expect (event.userGroup == 1);
                  expect (event.group == 0);
                  expect (event.status == UmpValues::ChannelVoice::noteOn);
                  expect (event.userChannel == 2);
                  expect (event.channel == 1);
                  expect (event.note == 64);
                  expect (event.velocity == 127);
              });

        test ("note off - roundtrip",
              [&] ()
              {
                  Midi1NoteOffEvent event (1, 2, 60, 100);
                  juce::ValueTree valueTree = event;
                  expect (valueTree.isValid ());
                  UmpEvent umpEvent (valueTree);
                  Midi1NoteOffEvent event2 (umpEvent);
                  expect (event2.userGroup == 1);
                  expect (event2.group == 0);
                  expect (event2.status == UmpValues::ChannelVoice::noteOff);
                  expect (event2.userChannel == 2);
                  expect (event2.channel == 1);
                  expect (event2.note == 60);
                  expect (event2.velocity == 100);
              });

        test ("poly pressure",
              [&] ()
              {
                  Midi1PolyPressureEvent event (1, 2, 60, 64);
                  expect (event.userGroup == 1);
                  expect (event.group == 0);
                  expect (event.status == UmpValues::ChannelVoice::polyPressure);
                  expect (event.userChannel == 2);
                  expect (event.channel == 1);
                  expect (event.note == 60);
                  expect (event.pressure == 64);
              });

        test ("control change",
              [&] ()
              {
                  Midi1ControlChangeEvent event (1, 2, 7, 100);
                  expect (event.userGroup == 1);
                  expect (event.group == 0);
                  expect (event.status == UmpValues::ChannelVoice::controlChange);
                  expect (event.userChannel == 2);
                  expect (event.channel == 1);
                  expect (event.controller == 7);
                  expect (event.value == 100);
              });

        test ("program change",
              [&] ()
              {
                  Midi1ProgramChangeEvent event (1, 2, 42);
                  expect (event.userGroup == 1);
                  expect (event.group == 0);
                  expect (event.status == UmpValues::ChannelVoice::programChange);
                  expect (event.userChannel == 2);
                  expect (event.channel == 1);
                  expect (event.program == 42);
              });

        test ("channel pressure",
              [&] ()
              {
                  Midi1ChannelPressureEvent event (1, 2, 90);
                  expect (event.userGroup == 1);
                  expect (event.group == 0);
                  expect (event.status == UmpValues::ChannelVoice::channelPressure);
                  expect (event.userChannel == 2);
                  expect (event.channel == 1);
                  expect (event.pressure == 90);
              });

        test ("pitch bend",
              [&] ()
              {
                  Midi1PitchBendEvent event (1, 2, MidiWord (8192));
                  expect (event.userGroup == 1);
                  expect (event.group == 0);
                  expect (event.status == UmpValues::ChannelVoice::pitchBend);
                  expect (event.userChannel == 2);
                  expect (event.channel == 1);
                  expect (event.lsb == 0);
                  expect (event.msb == 64);
                  expect (event.value == 8192);

                  Midi1PitchBendEvent event2 (1, 2, MidiWord (0));
                  expect (event2.lsb == 0);
                  expect (event2.msb == 0);
                  expect (event2.value == 0);

                  Midi1PitchBendEvent event3 (1, 2, MidiWord (16383));
                  expect (event3.lsb == 127);
                  expect (event3.msb == 127);
                  expect (event3.value == 16383);
              });
    }

private:
    // !!! test class member vars here...
};

static Test_ChannelVoice1 testChannelVoice1;
