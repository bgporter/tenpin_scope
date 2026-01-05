

#include <juce_core/juce_core.h>

class Test_Palette : public TestSuite
{
public:
    Test_Palette ()
    : TestSuite ("Palette", "View")
    {
    }

    void runTest () override
    {
        test ("color cascade",
              [this] ()
              {
                  cello::Object root { "root", nullptr };
                  Palette palette { root };
                  palette.menuText = PaletteColor ("defaultText");
                  expect (static_cast<juce::Colour> (palette.menuText.get ()) ==
                          static_cast<juce::Colour> (palette.defaultText.get ()));

                  // cascades should only go 1 level deep!
                  palette.highlightedText = PaletteColor ("menuText");
                  expect (static_cast<juce::Colour> (palette.highlightedText.get ()) !=
                          static_cast<juce::Colour> (palette.defaultText.get ()));
              });
    }

private:
    // !!! test class member vars here...
};

static Test_Palette testPalette;
