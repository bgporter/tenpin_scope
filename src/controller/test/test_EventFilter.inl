

#include <juce_core/juce_core.h>

class Test_EventFilter : public TestSuite
{
public:
    Test_EventFilter ()
    : TestSuite ("EventFilter", "!!! category !!!")
    {
    }

    void runTest () override
    {
        test ("!!! test name !!!",
              [&] ()
              {
                  // !!! test code here...
              });
    }

private:
    // !!! test class member vars here...
};

static Test_EventFilter testEventFilter;
