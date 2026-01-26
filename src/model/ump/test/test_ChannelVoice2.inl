

#include <juce_core/juce_core.h>

class Test_ChannelVoice2 : public TestSuite
{
public:
    Test_ChannelVoice2 ()
    : TestSuite ("ChannelVoice2", "!!! category !!!")
    {
    }

    void runTest () override
    {
        /*
          To create a test, call `test("testName", testLambda);`
          To (temporarily) skip a test, call `skipTest("testName", testLambda);`
          To define setup for a block of tests, call `setup(setupLambda);`
          To define cleanup for a block of tests, call `tearDown(tearDownLambda);`

          Setup and TearDown lambdas will be called before/after each test that
          is executed, and remain in effect until explicitly replaced.

          All the functionality of the JUCE `UnitTest` class is available from
          within these tests.
        */
        test ("note off - simple creation",
              [&] ()
              {
                  Midi2NoteOffEvent noteOffEvent (1, 2, 60, (uint16_t) 100);
                  expect (noteOffEvent.group == 1);
                  expect (noteOffEvent.status == UmpValues::noteOff);
                  expect (noteOffEvent.channel == 2);
                  expect (noteOffEvent.note == 60);
                  expect (noteOffEvent.velocity == 100);
                  expect (noteOffEvent.attributeType == 0);
                  expect (noteOffEvent.attributeValue == 0);
                  noteOffEvent.attributeType  = 1;
                  noteOffEvent.attributeValue = 100;
                  expect (noteOffEvent.attributeType == 1);
                  expect (noteOffEvent.attributeValue == 100);
              });
        test ("note off - roundtrip",
              [&] ()
              {
                  Midi2NoteOffEvent noteOffEvent (1, 2, 60,(uint16_t)  100);
                  juce::ValueTree valueTree = noteOffEvent;
                  expect (valueTree.isValid ());
                  DBG (valueTree.toXmlString ());
                  UmpEvent umpEvent (valueTree);
                  Midi2NoteOffEvent noteOffEvent2 (umpEvent);
                  expect (noteOffEvent2.group == 1);
                  expect (noteOffEvent2.status == UmpValues::noteOff);
                  expect (noteOffEvent2.channel == 2);
                  expect (noteOffEvent2.note == 60);
                  expect (noteOffEvent2.velocity == 100);
                  expect (noteOffEvent2.attributeType == 0);
                  expect (noteOffEvent2.attributeValue == 0);
              });

        test ("note off -- velocity conversion",
              [&] ()
              {
                  Midi2NoteOffEvent noteOffEvent (1, 2, 60, (uint16_t) 100);
                  expectWithinAbsoluteError (noteOffEvent.velocityFloat.get (), 100.0f / 65535.0f, 0.0001f);
                  noteOffEvent.velocityFloat = 0.5f;
                  expect (noteOffEvent.velocity == 32768);
                  expectWithinAbsoluteError (noteOffEvent.velocityFloat.get (), 0.5f, 0.0001f);
              });

        test ("note off - float velocity",
              [&] ()
              {
                  Midi2NoteOffEvent noteOffEvent (1, 2, 60, MidiUnipolarFloat(0.5f));
                  expect (noteOffEvent.velocity == 32768);

                  // create a note off event with the velocity as an int and verify that the values
                  // match between the two, and that the underlying value trees are the same
                  // using the ValueTree::isEquivalentTo method.
                  Midi2NoteOffEvent noteOffEvent2 (1, 2, 60, (uint16_t) 32768);
                  juce::ValueTree tree1 { noteOffEvent };
                  juce::ValueTree tree2 { noteOffEvent2 };
                  expect (tree1.isEquivalentTo (tree2));
              });

        test ("assignable per-note controller",
              [&] ()
              {
                  Midi2AssignablePerNoteControllerEvent event (1, 2, 60,  5, 0x12345678);
                  expect (event.group == 1);
                  expect (event.status == UmpValues::assignablePerNoteController);
                  expect (event.channel == 2);
                  expect (event.note == 60);
                  expect (event.controller == 5);
                  expect (event.value == 0x12345678);

                  Midi2AssignablePerNoteControllerEvent event2 (1, 2, 60,  5, MidiUnipolarFloat (0.5f));
                  expect (event2.value == 0x80000000);
                  expectWithinAbsoluteError (event2.valueFloat.get (), 0.5f, 0.0001f);
              });

        test ("registered controller",
              [&] ()
              {
                  Midi2RegisteredControllerEvent event (1, 2, 0, 1, 0x12345678);
                  expect (event.group == 1);
                  expect (event.status == UmpValues::registeredController);
                  expect (event.channel == 2);
                  expect (event.bank == 0);
                  expect (event.controller == 1);
                  expect (event.value == 0x12345678);
              });

        test ("assignable controller",
              [&] ()
              {
                  Midi2AssignableControllerEvent event (1, 2, 127, 2, 0x80000000);
                  expect (event.group == 1);
                  expect (event.status == UmpValues::assignableController);
                  expect (event.channel == 2);
                  expect (event.bank == 127);
                  expect (event.controller == 2);
                  expect (event.value == 0x80000000);
              });

        test ("relative registered controller",
              [&] ()
              {
                  Midi2RelativeRegisteredControllerEvent event (1, 2, 0, 1, (int32_t) -100);
                  expect (event.group == 1);
                  expect (event.status == UmpValues::relativeRegisteredController);
                  expect (event.channel == 2);
                  expect (event.bank == 0);
                  expect (event.controller == 1);
                  expect (event.value == -100);

                  Midi2RelativeRegisteredControllerEvent event2 (1, 2, 0, 1, MidiBipolarFloat (-1.0f));
                  expect (event2.value == -2147483648);
                  expectWithinAbsoluteError (event2.valueFloat.get (), -1.0f, 0.0001f);
              });

        test ("relative assignable controller",
              [&] ()
              {
                  Midi2RelativeAssignableControllerEvent event (1, 2, 127, 2, (int32_t) 100);
                  expect (event.group == 1);
                  expect (event.status == UmpValues::relativeAssignableController);
                  expect (event.channel == 2);
                  expect (event.bank == 127);
                  expect (event.controller == 2);
                  expect (event.value == 100);

                  Midi2RelativeAssignableControllerEvent event2 (1, 2, 127, 2, MidiBipolarFloat (1.0f));
                  expect (event2.value == 2147483647);
                  expectWithinAbsoluteError (event2.valueFloat.get (), 1.0f, 0.0001f);
              });

        test ("per-note pitch bend",
              [&] ()
              {
                  Midi2PerNotePitchBendEvent event (1, 2, 60, (int32_t) -100);
                  expect (event.group == 1);
                  expect (event.status == UmpValues::perNotePitchBend);
                  expect (event.channel == 2);
                  expect (event.note == 60);
                  expect (event.value == -100);

                  Midi2PerNotePitchBendEvent event2 (1, 2, 60, MidiBipolarFloat (1.0f));
                  expect (event2.value == 2147483647);
                  expectWithinAbsoluteError (event2.valueFloat.get (), 1.0f, 0.0001f);
              });

        test ("poly pressure",
              [&] ()
              {
                  Midi2PolyPressureEvent event (1, 2, 60, 0x12345678);
                  expect (event.group == 1);
                  expect (event.status == UmpValues::polyPressure);
                  expect (event.channel == 2);
                  expect (event.note == 60);
                  expect (event.value == 0x12345678);

                  Midi2PolyPressureEvent event2 (1, 2, 60, MidiUnipolarFloat (1.0f));
                  expect (event2.value == 0xFFFFFFFF);
                  expectWithinAbsoluteError (event2.valueFloat.get (), 1.0f, 0.0001f);
              });

        test ("channel pressure",
              [&] ()
              {
                  Midi2ChannelPressureEvent event (1, 2, 0x12345678);
                  expect (event.group == 1);
                  expect (event.status == UmpValues::channelPressure);
                  expect (event.channel == 2);
                  expect (event.value == 0x12345678);

                  Midi2ChannelPressureEvent event2 (1, 2, MidiUnipolarFloat (1.0f));
                  expect (event2.value == 0xFFFFFFFF);
                  expectWithinAbsoluteError (event2.valueFloat.get (), 1.0f, 0.0001f);
              });
    }

private:
    // !!! test class member vars here...
};

static Test_ChannelVoice2 testChannelVoice2;
