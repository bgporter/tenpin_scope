

#include <juce_core/juce_core.h>

class Test_Sysex7 : public TestSuite
{
public:
    Test_Sysex7 ()
    : TestSuite ("Sysex7", "!!! category !!!")
    {
    }

    void runTest () override
    {
        using namespace midi_literals;

        test ("individual-byte construction",
              [&] ()
              {
                  Sysex7Event event (1_gr, SysexStatus::complete, 4, 0x7E, 0x01, 0x06, 0x01);
                  expect (event.userGroup == 1);
                  expect (event.group == 0);
                  expect (event.messageType == MessageTypes::sysex7);
                  expect (event.status == SysexStatus::complete);
                  expect (event.numBytes == 4);
                  expect (event[0] == 0x7E);
                  expect (event[1] == 0x01);
                  expect (event[2] == 0x06);
                  expect (event[3] == 0x01);
                  expect (event[4] == 0);
                  expect (event[5] == 0);
              });

        test ("span construction",
              [&] ()
              {
                  const uint8_t buf[] = { 0x41, 0x10, 0x42, 0x12 };
                  Sysex7Event event (2_gr, SysexStatus::start, std::span (buf));
                  expect (event.userGroup == 2);
                  expect (event.status == SysexStatus::start);
                  expect (event.numBytes == 4);
                  expect (event[0] == 0x41);
                  expect (event[1] == 0x10);
                  expect (event[2] == 0x42);
                  expect (event[3] == 0x12);
              });

        test ("span clamped to 6 bytes",
              [&] ()
              {
                  const uint8_t buf[] = { 1, 2, 3, 4, 5, 6, 7, 8 };
                  Sysex7Event event (1_gr, SysexStatus::continue_, std::span (buf));
                  expect (event.numBytes == 6);
                  expect (event[5] == 6);
              });

        test ("7-bit masking in span constructor",
              [&] ()
              {
                  const uint8_t buf[] = { 0xFF, 0x80 };
                  Sysex7Event event (1_gr, SysexStatus::complete, std::span (buf));
                  expect (event[0] == 0x7F);
                  expect (event[1] == 0x00);
              });

        test ("operator[] write via non-const ref",
              [&] ()
              {
                  Sysex7Event event (1_gr, SysexStatus::complete, 3, 10, 20, 30);
                  event[1] = 99;
                  expect (event[1] == 99);
                  expect (event[0] == 10);
                  expect (event[2] == 30);
              });

        test ("all status values",
              [&] ()
              {
                  expect (Sysex7Event (1_gr, SysexStatus::complete,  0).status == SysexStatus::complete);
                  expect (Sysex7Event (1_gr, SysexStatus::start,     0).status == SysexStatus::start);
                  expect (Sysex7Event (1_gr, SysexStatus::continue_, 0).status == SysexStatus::continue_);
                  expect (Sysex7Event (1_gr, SysexStatus::end,       0).status == SysexStatus::end);
              });

        test ("factory: 3-byte span → complete",
              [&] ()
              {
                  const uint8_t buf[] = { 0x7E, 0x7F, 0x06 };
                  int count = 0;
                  Sysex7EventFactory factory ([&] (const Sysex7Event& e)
                  {
                      ++count;
                      expect (e.status == SysexStatus::complete);
                      expect (e.numBytes == 3);
                      expect (e[0] == 0x7E);
                      expect (e[2] == 0x06);
                  });
                  factory.createEvents (1_gr, std::span (buf));
                  expect (count == 1);
              });

        test ("factory: 6-byte span → complete, numBytes 6",
              [&] ()
              {
                  const uint8_t buf[] = { 1, 2, 3, 4, 5, 6 };
                  int count = 0;
                  Sysex7EventFactory factory ([&] (const Sysex7Event& e)
                  {
                      ++count;
                      expect (e.status == SysexStatus::complete);
                      expect (e.numBytes == 6);`
                  });
                  factory.createEvents (1_gr, std::span (buf));
                  expect (count == 1);
              });

        test ("factory: 7-byte span → start + end",
              [&] ()
              {
                  const uint8_t buf[] = { 1, 2, 3, 4, 5, 6, 7 };
                  int count = 0;
                  Sysex7EventFactory factory ([&] (const Sysex7Event& e)
                  {
                      if (count == 0)
                      {
                          expect (e.status == SysexStatus::start);
                          expect (e.numBytes == 6);
                      }
                      else if (count == 1)
                      {
                          expect (e.status == SysexStatus::end);
                          expect (e.numBytes == 1);
                          expect (e[0] == 7);
                      }
                      ++count;
                  });
                  factory.createEvents (1_gr, std::span (buf));
                  expect (count == 2);
              });

        test ("factory: 12-byte span → start + end, both 6 bytes",
              [&] ()
              {
                  const uint8_t buf[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };
                  int count = 0;
                  Sysex7EventFactory factory ([&] (const Sysex7Event& e)
                  {
                      if (count == 0)
                      {
                          expect (e.status == SysexStatus::start);
                          expect (e.numBytes == 6);
                      }
                      else if (count == 1)
                      {
                          expect (e.status == SysexStatus::end);
                          expect (e.numBytes == 6);
                      }
                      ++count;
                  });
                  factory.createEvents (1_gr, std::span (buf));
                  expect (count == 2);
              });

        test ("factory: 13-byte span → start + continue + end",
              [&] ()
              {
                  const uint8_t buf[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };
                  int count = 0;
                  Sysex7EventFactory factory ([&] (const Sysex7Event& e)
                  {
                      if (count == 0)
                      {
                          expect (e.status == SysexStatus::start);
                          expect (e.numBytes == 6);
                      }
                      else if (count == 1)
                      {
                          expect (e.status == SysexStatus::continue_);
                          expect (e.numBytes == 6);
                      }
                      else if (count == 2)
                      {
                          expect (e.status == SysexStatus::end);
                          expect (e.numBytes == 1);
                          expect (e[0] == 13);
                      }
                      ++count;
                  });
                  factory.createEvents (1_gr, std::span (buf));
                  expect (count == 3);
              });

        test ("factory: null handler → no crash",
              [&] ()
              {
                  const uint8_t buf[] = { 1, 2, 3 };
                  Sysex7EventFactory factory (nullptr);
                  factory.createEvents (1_gr, std::span (buf));
                  expect (true);
              });

        test ("roundtrip via ValueTree",
              [&] ()
              {
                  Sysex7Event event (1_gr, SysexStatus::complete, 3, 0x7E, 0x01, 0x06);
                  juce::ValueTree valueTree = event;
                  expect (valueTree.isValid ());
                  UmpEvent umpEvent (valueTree);
                  Sysex7Event event2 (umpEvent);
                  expect (event2.userGroup == 1);
                  expect (event2.status == SysexStatus::complete);
                  expect (event2.numBytes == 3);
                  expect (event2[0] == 0x7E);
                  expect (event2[1] == 0x01);
                  expect (event2[2] == 0x06);
              });
    }

private:
    // !!! test class member vars here...
};

static Test_Sysex7 testSysex7;
