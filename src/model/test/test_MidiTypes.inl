

#include <juce_core/juce_core.h>

class Test_MidiTypes : public TestSuite
{
public:
    Test_MidiTypes ()
    : TestSuite ("MidiTypes", "!!! category !!!")
    {
    }

    void runTest () override
    {
        test ("MidiWord - lsb/msb constructor",
              [&] ()
              {
                  MidiByte lsb (0x7F);
                  MidiByte msb (0x7F);
                  MidiWord word (lsb, msb);
                  expect (word.get () == 16383);
                  expect (word.getLsb () == 0x7F);
                  expect (word.getMsb () == 0x7F);

                  MidiWord word2 (MidiByte (0x01), MidiByte (0x02));
                  expect (word2.get () == (2 << 7) | 1);
                  expect (word2.getLsb () == 1);
                  expect (word2.getMsb () == 2);
              });

        test ("MidiLong - 4 byte constructor",
              [&] ()
              {
                  MidiByte b1 (0x01);
                  MidiByte b2 (0x02);
                  MidiByte b3 (0x03);
                  MidiByte b4 (0x04);
                  MidiLong q (b1, b2, b3, b4);
                  expect (q.get () == (4 << 21) | (3 << 14) | (2 << 7) | 1);
                  expect (q.getLsb () == 1);
                  expect (q.getByte2 () == 2);
                  expect (q.getByte3 () == 3);
                  expect (q.getMsb () == 4);

                  MidiLong q2 (MidiByte (0x7F), MidiByte (0x7F), MidiByte (0x7F), MidiByte (0x7F));
                  expect (q2.get () == 268435455);
              });
    }

private:
    // !!! test class member vars here...
};

static Test_MidiTypes testMidiTypes;
