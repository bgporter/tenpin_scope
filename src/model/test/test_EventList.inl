

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

        test ("reset - onChildrenCleared fires when same object clears",
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

                  el.onChildrenCleared = [&] () { wasReset = true; };
                  el.clear ();
                  expect (wasReset);
                  expect (el.getNumChildren () == 0);
              });

        test ("reset - onChildrenCleared fires on listener when other object clears same tree",
              [&] ()
              {
                  EventList el;
                  for (int i = 0; i < 10; i++)
                  {
                      EventList child;
                      el.append (&child);
                  }
                  expect (el.getNumChildren () == 10);

                  EventList elCopy { el };
                  elCopy.onChildrenCleared = [&] () { wasReset = true; };

                  el.clear ();

                  expect (wasReset);
                  expect (el.getNumChildren () == 0);
                  expect (elCopy.getNumChildren () == 0);
              });
    }

private:
    // !!! test class member vars here...
    bool wasReset { false };
};

static Test_EventList testEventList;
