/*
 MIT License

 Copyright (c) 2026 Brett g Porter

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#pragma once

#include <JuceHeader.h>

#include "logger.h"

/**
 * @class PaletteColor
 *
 * @brief a helpful way to represent colors as text. Intended to be used
 * to create human-editable palette files.
 *
 * Understands these encodings:
 *
 * - #RRGGBB (implied alpha = max)
 * - #RGB (implied alpha = max)
 * - #RRGGBBAA
 * - named colors (e.g. "red", "green", "blue", etc.)
 *
 * Invalid colors are returned as 0xFFFF00FF (invalidColor) which is a bright magenta.

 * Instantiate by passing in a string that represents the color, and access it as a juce::Colour
 * which will perform the conversion lazily.
 */

class PaletteColor
{
public:
    static const inline juce::Colour invalidColor { 0xFFFF00FF };
    PaletteColor (juce::String theRepresentation)
    : representation (theRepresentation)
    {
    }

    operator juce::Colour () const
    {
        if (representation.startsWith ("#"))
        {
            // one of the RGB formats we support. Convert into a standard 8-byte
            // ARGB format, then pass to our internal parse function.
            const auto colorString = representation.substring (1);
            const wchar_t F        = 'F';
            switch (colorString.length ())
            {
                case 3:
                {
                    // 3-digit hex; set alpha to max
                    const wchar_t chars[] = { 'f',
                                              'f',
                                              colorString[0],
                                              colorString[0],
                                              colorString[1],
                                              colorString[1],
                                              colorString[2],
                                              colorString[2] };
                    juce::String representationExpanded { chars, 8 };
                    return parseRgba (representationExpanded);
                }
                case 6:
                {
                    // 6-digit hex; set alpha to max
                    return parseRgba (juce::String { "FF" } + colorString);
                }
                case 8:
                {
                    // 8-digit hex; convert from RGBA to ARGB
                    const auto rgb   = colorString.substring (0, 6);
                    const auto alpha = colorString.substring (6);
                    return parseRgba (alpha + rgb);
                }
                default:
                {
                    ERROR_ (juce::String ("Invalid color: ") + representation);
                    return invalidColor;
                }
            }
        }
        else
        {
            return parseNamed (representation);
        }
    }

    juce::String toString () const { return representation; }

private:
    static juce::Colour parseRgba (juce::String argb)
    {
        return juce::Colour { static_cast<juce::uint32> (argb.getHexValue32 ()) };
    }
    static juce::Colour parseNamed (juce::String name) { return juce::Colours::findColourForName (name, invalidColor); }

    juce::String representation;
};

namespace juce
{
template <> struct VariantConverter<PaletteColor>
{
    static PaletteColor fromVar (const juce::var& v) { return PaletteColor { v.toString () }; }
    static juce::var toVar (const PaletteColor& c) { return c.toString (); }
};
} // namespace juce
