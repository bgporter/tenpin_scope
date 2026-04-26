

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
                  // lower quarter: 32/127 ~= 0.252
                  expectWithinAbsoluteError (
                      formatValue (32u, 7, ValueFormatType::Float).getFloatValue(), 0.252f, 0.01f);
                  // upper quarter: 96/127 ~= 0.756
                  expectWithinAbsoluteError (
                      formatValue (96u, 7, ValueFormatType::Float).getFloatValue(), 0.756f, 0.01f);
              });

        test ("float formatting - unipolar 32-bit",
              [this] ()
              {
                  expectEquals (formatValue (0x00000000u, 32, ValueFormatType::Float), juce::String ("MIN"));
                  expectEquals (formatValue (0x80000000u, 32, ValueFormatType::Float), juce::String ("MID"));
                  expectEquals (formatValue (0xFFFFFFFFu, 32, ValueFormatType::Float), juce::String ("MAX"));
                  // lower quarter: 0x40000000 / 0xFFFFFFFF ~= 0.25
                  expectWithinAbsoluteError (
                      formatValue (0x40000000u, 32, ValueFormatType::Float).getFloatValue(), 0.25f, 0.001f);
                  // upper quarter: 0xC0000000 / 0xFFFFFFFF ~= 0.75
                  expectWithinAbsoluteError (
                      formatValue (0xC0000000u, 32, ValueFormatType::Float).getFloatValue(), 0.75f, 0.001f);
              });

        test ("float formatting - bipolar 32-bit",
              [this] ()
              {
                  expectEquals (formatValue (0x00000000u, 32, ValueFormatType::Float, 2, -1.f, 1.f), juce::String ("MIN"));
                  expectEquals (formatValue (0x80000000u, 32, ValueFormatType::Float, 2, -1.f, 1.f), juce::String ("MID"));
                  expectEquals (formatValue (0xFFFFFFFFu, 32, ValueFormatType::Float, 2, -1.f, 1.f), juce::String ("MAX"));
                  // lower quarter: -1 + 2 * (0x40000000/0xFFFFFFFF) ~= -0.50
                  expectWithinAbsoluteError (
                      formatValue (0x40000000u, 32, ValueFormatType::Float, 2, -1.f, 1.f).getFloatValue(), -0.5f, 0.001f);
                  // upper quarter: -1 + 2 * (0xC0000000/0xFFFFFFFF) ~= 0.50
                  expectWithinAbsoluteError (
                      formatValue (0xC0000000u, 32, ValueFormatType::Float, 2, -1.f, 1.f).getFloatValue(), 0.5f, 0.001f);
              });

        test ("midi formatting - unipolar 7-bit",
              [this] ()
              {
                  expectEquals (formatValue (0u,   7, ValueFormatType::Midi), juce::String ("0"));
                  expectEquals (formatValue (32u,  7, ValueFormatType::Midi), juce::String ("32"));
                  expectEquals (formatValue (64u,  7, ValueFormatType::Midi), juce::String ("64"));
                  expectEquals (formatValue (96u,  7, ValueFormatType::Midi), juce::String ("96"));
                  expectEquals (formatValue (127u, 7, ValueFormatType::Midi), juce::String ("127"));
              });

        test ("midi formatting - unipolar 32-bit",
              [this] ()
              {
                  // minimum: int=0, frac=0
                  expectEquals (formatValue (0x00000000u, 32, ValueFormatType::Midi), juce::String ("0.00"));
                  // top 7 bits of 0x80000000 = 1000000b = 64; fraction = 0
                  expectEquals (formatValue (0x80000000u, 32, ValueFormatType::Midi), juce::String ("64.00"));
                  // maximum: top 7 bits = 127, fraction approaches 1.0
                  expect (formatValue (0xFFFFFFFFu, 32, ValueFormatType::Midi).startsWith ("127."));
                  // 0x51000000: top 7 bits = 40, low 25 bits = 0x01000000 = 0.5 of 2^25
                  expectEquals (formatValue (0x51000000u, 32, ValueFormatType::Midi), juce::String ("40.50"));
              });

        test ("percent formatting - unipolar",
              [this] ()
              {
                  expectEquals (formatValue (0u,   7, ValueFormatType::Percent), juce::String ("0.00%"));
                  expectEquals (formatValue (127u, 7, ValueFormatType::Percent), juce::String ("100.00%"));
                  // 32/127 ~= 25.20%
                  expectWithinAbsoluteError (
                      formatValue (32u, 7, ValueFormatType::Percent).dropLastCharacters (1).getFloatValue(),
                      25.2f, 0.1f);
                  // 64/127 ~= 50.39%
                  expectWithinAbsoluteError (
                      formatValue (64u, 7, ValueFormatType::Percent).dropLastCharacters (1).getFloatValue(),
                      50.4f, 0.1f);
              });

        test ("percent formatting - bipolar 32-bit",
              [this] ()
              {
                  expectEquals (formatValue (0x00000000u, 32, ValueFormatType::Percent, 2, -1.f, 1.f), juce::String ("-100.00%"));
                  expectEquals (formatValue (0xFFFFFFFFu, 32, ValueFormatType::Percent, 2, -1.f, 1.f), juce::String ("100.00%"));
                  // 0x80000000 is midpoint: -1 + 2 * 0.5 = 0
                  expectWithinAbsoluteError (
                      formatValue (0x80000000u, 32, ValueFormatType::Percent, 2, -1.f, 1.f).dropLastCharacters (1).getFloatValue(),
                      0.f, 0.001f);
                  // 0x40000000 is lower quarter: -1 + 2 * 0.25 = -0.50 -> -50%
                  expectWithinAbsoluteError (
                      formatValue (0x40000000u, 32, ValueFormatType::Percent, 2, -1.f, 1.f).dropLastCharacters (1).getFloatValue(),
                      -50.f, 0.01f);
              });

        test ("midi formatting - bipolar 32-bit",
              [this] ()
              {
                  // formattedMin = -64 shifts integer part so 0x00000000 -> -64 and 0x80000000 -> 0
                  expectEquals (formatValue (0x00000000u, 32, ValueFormatType::Midi, 2, -64.f, 1.f), juce::String ("-64.00"));
                  expectEquals (formatValue (0x80000000u, 32, ValueFormatType::Midi, 2, -64.f, 1.f), juce::String ("0.00"));
                  expect (formatValue (0xFFFFFFFFu, 32, ValueFormatType::Midi, 2, -64.f, 1.f).startsWith ("63."));
                  // 0x51000000: top 7 bits = 40, offset by -64 = -24; low 25 bits = 0.5 of 2^25
                  expectEquals (formatValue (0x51000000u, 32, ValueFormatType::Midi, 2, -64.f, 1.f), juce::String ("-24.50"));
              });
    }

private:
};

static Test_EventNameUtils testEventNameUtils;
