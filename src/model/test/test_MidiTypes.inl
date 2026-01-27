

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

        test ("MidiUnipolarFloat - conversions",
              [&] ()
              {
                  MidiUnipolarFloat f1 (0.0f);
                  expect (f1.toUint32 () == 0);
                  expect (MidiUnipolarFloat::fromUint32 (0) == 0.0f);

                  MidiUnipolarFloat f2 (1.0f);
                  expect (f2.toUint32 () == 0xFFFFFFFF);
                  expect (MidiUnipolarFloat::fromUint32 (0xFFFFFFFF) == 1.0f);

                  MidiUnipolarFloat f3 (0.5f);
                  // 0.5 * 4294967295 = 2147483647.5 -> rounded to 2147483648
                  expect (f3.toUint32 () == 2147483648);

                  MidiUnipolarFloat f4 (1.0f);
                  expect (f4.toUint16 () == 65535);
                  expect (MidiUnipolarFloat::fromUint16 (65535) == 1.0f);
              });

        test ("MidiBipolarFloat - conversions",
              [&] ()
              {
                  MidiBipolarFloat f1 (0.0f);
                  expect (f1.toInt32 () == 0);
                  expect (MidiBipolarFloat::fromInt32 (0) == 0.0f);

                  MidiBipolarFloat f2 (1.0f);
                  expect (f2.toInt32 () == 2147483647);
                  expect (MidiBipolarFloat::fromInt32 (2147483647) == 1.0f);

                  MidiBipolarFloat f3 (-1.0f);
                  expect (f3.toInt32 () == -2147483648);
                  expect (MidiBipolarFloat::fromInt32 ((int32_t) -2147483648) == -1.0f);
              });

        test ("Literals",
              [&] ()
              {
                  using namespace midi_literals;
                  MidiGroup g = 5_gr;
                  expect (g.get () == 5);

                  MidiChannel c = 10_ch;
                  expect (c.get () == 10);
              });
    }

private:
    // !!! test class member vars here...
};

static Test_MidiTypes testMidiTypes;
