

#include <juce_core/juce_core.h>

class Test_PaletteColor : public TestSuite
{
public:
    Test_PaletteColor ()
    : TestSuite ("PaletteColor", "utility")
    {
    }

    bool testColorConversion (juce::StringRef spec, juce::Colour color)
    {
        PaletteColor pc (spec);
        return (pc.toString () == spec) && color == pc;
    }

    void runTest () override
    {
        beginTest ("PaletteColor");

        test ("3-digit hex",
              [this] ()
              {
                  expect (testColorConversion ("#FFF", juce::Colours::white));
                  expect (testColorConversion ("#000", juce::Colours::black));
                  expect (testColorConversion ("#F0F", juce::Colours::fuchsia));
                  expect (testColorConversion ("#0F0", juce::Colours::lime));
                  expect (testColorConversion ("#00F", juce::Colours::blue));
                  expect (testColorConversion ("#F00", juce::Colours::red));
              });

        test ("6-digit hex",
              [this] ()
              {
                  expect (testColorConversion ("#FFFFFF", juce::Colours::white));
                  expect (testColorConversion ("#000000", juce::Colours::black));
                  expect (testColorConversion ("#FF00FF", juce::Colours::fuchsia));
                  expect (testColorConversion ("#00FF00", juce::Colours::lime));
                  expect (testColorConversion ("#0000FF", juce::Colours::blue));
                  expect (testColorConversion ("#FF0000", juce::Colours::red));
              });

        test ("8-digit hex",
              [this] ()
              {
                  // NOTE that these are all in (CSS-style) RGBA order, not ARGB
                  expect (testColorConversion ("#FFFFFFFF", juce::Colours::white));
                  expect (testColorConversion ("#000000FF", juce::Colours::black));
                  expect (testColorConversion ("#FF00FFFF", juce::Colours::fuchsia));
                  expect (testColorConversion ("#00FF00FF", juce::Colours::lime));
                  expect (testColorConversion ("#0000FFFF", juce::Colours::blue));
                  expect (testColorConversion ("#FF0000FF", juce::Colours::red));

                  juce::uint8 alpha = 0x80;
                  expect (testColorConversion ("#FFFFFF80", juce::Colours::white.withAlpha (alpha)));
                  expect (testColorConversion ("#00000080", juce::Colours::black.withAlpha (alpha)));
                  expect (testColorConversion ("#FF00FF80", juce::Colours::fuchsia.withAlpha (alpha)));
                  expect (testColorConversion ("#00FF0080", juce::Colours::lime.withAlpha (alpha)));
                  expect (testColorConversion ("#0000FF80", juce::Colours::blue.withAlpha (alpha)));
                  expect (testColorConversion ("#FF000080", juce::Colours::red.withAlpha (alpha)));

                  alpha = 0;
                  expect (testColorConversion ("#FFFFFF00", juce::Colours::white.withAlpha (alpha)));
                  expect (testColorConversion ("#00000000", juce::Colours::black.withAlpha (alpha)));
                  expect (testColorConversion ("#FF00FF00", juce::Colours::fuchsia.withAlpha (alpha)));
                  expect (testColorConversion ("#00FF0000", juce::Colours::lime.withAlpha (alpha)));
                  expect (testColorConversion ("#0000FF00", juce::Colours::blue.withAlpha (alpha)));
                  expect (testColorConversion ("#FF000000", juce::Colours::red.withAlpha (alpha)));
              });

        test ("named",
              [this] ()
              {
                  expect (testColorConversion ("white", juce::Colours::white));
                  expect (testColorConversion ("black", juce::Colours::black));
                  expect (testColorConversion ("fuchsia", juce::Colours::fuchsia));
                  expect (testColorConversion ("lime", juce::Colours::lime));
                  expect (testColorConversion ("blue", juce::Colours::blue));
                  expect (testColorConversion ("red", juce::Colours::red));
                  expect (testColorConversion ("invalid", PaletteColor::invalidColor));
              });
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
    }

private:
    // !!! test class member vars here...
};

static Test_PaletteColor testPaletteColor;
