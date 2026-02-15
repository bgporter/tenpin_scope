

#include <juce_core/juce_core.h>

class Test_EventList : public TestSuite
{
public:
    Test_EventList ()
    : TestSuite ("EventList", "!!! category !!!")
    {
    }

    void runTest () override
    {
        setup ([&] () { wasReset = false; });
        test ("basic initialization",
              [&] ()
              {
                  EventList el;
                  expect (el.getType ().toString () == "EventList");
                  expect (el.getNumChildren () == 0);
              });

        test ("reset",
              [&] ()
              {
                  EventList el;
                  expect (el.getType ().toString () == "EventList");
                  expect (el.getNumChildren () == 0);

                  for (int i = 0; i < 10; i++)
                  {
                      EventList child;
                      el.append (&child);
                  }
                  expect (el.getNumChildren () == 10);

                  el.onTreeRedirected = [&] () { wasReset = true; };
                  el.clear ();
                  expect (wasReset);
                  expect (el.getNumChildren () == 0);
              });
    }

private:
    // !!! test class member vars here...
    bool wasReset { false };
};

static Test_EventList testEventList;
