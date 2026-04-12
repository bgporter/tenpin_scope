

#include <juce_core/juce_core.h>

class Test_EventNameUtils : public TestSuite
{
public:
    Test_EventNameUtils ()
    : TestSuite ("EventNameUtils", "View")
    {
    }

    void runTest () override
    {
        test ("hex formatting - 8-bit",
              [this] ()
              {
                  expectEquals (formatValue (0x00u, 8, ValueFormatType::Hex), juce::String ("0x00"));
                  expectEquals (formatValue (0x40u, 8, ValueFormatType::Hex), juce::String ("0x40"));
                  expectEquals (formatValue (0xFFu, 8, ValueFormatType::Hex), juce::String ("0xff"));
              });

        test ("hex formatting - 16-bit",
              [this] ()
              {
                  expectEquals (formatValue (0x0000u, 16, ValueFormatType::Hex), juce::String ("0x0000"));
                  expectEquals (formatValue (0x8000u, 16, ValueFormatType::Hex), juce::String ("0x8000"));
                  expectEquals (formatValue (0xFFFFu, 16, ValueFormatType::Hex), juce::String ("0xffff"));
              });

        test ("hex formatting - 32-bit",
              [this] ()
              {
                  expectEquals (formatValue (0x00000000u, 32, ValueFormatType::Hex), juce::String ("0x00000000"));
                  expectEquals (formatValue (0x80000000u, 32, ValueFormatType::Hex), juce::String ("0x80000000"));
                  expectEquals (formatValue (0xFFFFFFFFu, 32, ValueFormatType::Hex), juce::String ("0xffffffff"));
              });

        test ("float formatting - unipolar 7-bit",
              [this] ()
              {
                  // special sentinel values
                  expectEquals (formatValue (0u,   7, ValueFormatType::Float), juce::String ("MIN"));
                  expectEquals (formatValue (64u,  7, ValueFormatType::Float), juce::String ("MID"));
                  expectEquals (formatValue (127u, 7, ValueFormatType::Float), juce::String ("MAX"));
                  // non-sentinel: 32/127 ~= 0.252
                  expectWithinAbsoluteError (
                      formatValue (32u, 7, ValueFormatType::Float).getFloatValue(), 0.252f, 0.01f);
              });

        test ("float formatting - unipolar 32-bit",
              [this] ()
              {
                  expectEquals (formatValue (0x00000000u, 32, ValueFormatType::Float), juce::String ("MIN"));
                  expectEquals (formatValue (0x80000000u, 32, ValueFormatType::Float), juce::String ("MID"));
                  expectEquals (formatValue (0xFFFFFFFFu, 32, ValueFormatType::Float), juce::String ("MAX"));
                  // non-sentinel: 0x40000000 ~= 0.25
                  expectWithinAbsoluteError (
                      formatValue (0x40000000u, 32, ValueFormatType::Float).getFloatValue(), 0.25f, 0.001f);
              });

        test ("float formatting - bipolar 32-bit",
              [this] ()
              {
                  expectEquals (formatValue (0x00000000u, 32, ValueFormatType::Float, 2, -1.f, 1.f), juce::String ("MIN"));
                  expectEquals (formatValue (0x80000000u, 32, ValueFormatType::Float, 2, -1.f, 1.f), juce::String ("MID"));
                  expectEquals (formatValue (0xFFFFFFFFu, 32, ValueFormatType::Float, 2, -1.f, 1.f), juce::String ("MAX"));
                  // non-sentinel: 0x40000000 ~= -0.50
                  expectWithinAbsoluteError (
                      formatValue (0x40000000u, 32, ValueFormatType::Float, 2, -1.f, 1.f).getFloatValue(), -0.5f, 0.001f);
              });

        test ("midi formatting - unipolar 7-bit",
              [this] ()
              {
                  expectEquals (formatValue (0u,   7, ValueFormatType::Midi), juce::String ("0"));
                  expectEquals (formatValue (64u,  7, ValueFormatType::Midi), juce::String ("64"));
                  expectEquals (formatValue (127u, 7, ValueFormatType::Midi), juce::String ("127"));
              });

        test ("midi formatting - unipolar 32-bit",
              [this] ()
              {
                  // top 7 bits of 0x80000000 = 1000000b = 64; fraction = 0
                  expectEquals (formatValue (0x80000000u, 32, ValueFormatType::Midi), juce::String ("64.00"));
                  // minimum: int=0, frac=0
                  expectEquals (formatValue (0x00000000u, 32, ValueFormatType::Midi), juce::String ("0.00"));
                  // maximum: top 7 bits = 127, fraction approaches 1.0
                  expect (formatValue (0xFFFFFFFFu, 32, ValueFormatType::Midi).startsWith ("127."));
              });

        test ("percent formatting - unipolar",
              [this] ()
              {
                  expectEquals (formatValue (0u,   7, ValueFormatType::Percent), juce::String ("0.00%"));
                  expectEquals (formatValue (127u, 7, ValueFormatType::Percent), juce::String ("100.00%"));
                  expectWithinAbsoluteError (
                      formatValue (64u, 7, ValueFormatType::Percent).dropLastCharacters (1).getFloatValue(),
                      50.4f, 0.1f);
              });

        test ("percent formatting - bipolar 32-bit",
              [this] ()
              {
                  expectEquals (formatValue (0x00000000u, 32, ValueFormatType::Percent, 2, -1.f, 1.f), juce::String ("-100.00%"));
                  expectEquals (formatValue (0xFFFFFFFFu, 32, ValueFormatType::Percent, 2, -1.f, 1.f), juce::String ("100.00%"));
                  expectWithinAbsoluteError (
                      formatValue (0x80000000u, 32, ValueFormatType::Percent, 2, -1.f, 1.f).dropLastCharacters (1).getFloatValue(),
                      0.f, 0.001f);
              });

        test ("midi formatting - bipolar 32-bit",
              [this] ()
              {
                  // formattedMin = -64 shifts integer part so 0x00000000 -> -64 and 0x80000000 -> 0
                  expectEquals (formatValue (0x00000000u, 32, ValueFormatType::Midi, 2, -64.f, 1.f), juce::String ("-64.00"));
                  expectEquals (formatValue (0x80000000u, 32, ValueFormatType::Midi, 2, -64.f, 1.f), juce::String ("0.00"));
                  expect (formatValue (0xFFFFFFFFu, 32, ValueFormatType::Midi, 2, -64.f, 1.f).startsWith ("63."));
              });
    }

private:
};

static Test_EventNameUtils testEventNameUtils;
