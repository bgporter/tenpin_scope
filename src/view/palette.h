/*
 MIT License

 Copyright (c) 2026 Brett g Porter

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to so, subject to the following conditions:

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

#include "utility/paletteColor.h"
#include "utility/variantConverters.h"

#ifdef COLOR_M
#undef COLOR_M
#endif

/**
 * @brief This class is going to define a bunch of PaletteColor
 *       members; this macro streamlines it.
 *
 * @param name The name of the property to define.
 * @param init The initial value of the property, as a string parseable by the PaletteColor class
 */
// clang-format off
#define COLOR_M(name, init) \
    MAKE_VALUE_MEMBER_GET(PaletteColor, name, PaletteColor{init}, checkReference)
// clang-format on

/**
 * @brief Define and manage the color palette for the application.
 *
 * Uses the PaletteColor class to define colors as strings in any of these formats:
 * - #RGB (implied alpha = max)
 * - #RRGGBB (implied alpha = max)
 * - #RRGGBBAA
 * - named colors (e.g. "red", "green", "blue", etc.)
 *
 * Invalid colors are returned as 0xFFFF00FF (invalidColor) which is a bright magenta.
 *
 * This class also supports a single level of indirection, so that you can define
 * a color as a reference to another color by its name in the palette.
 *
 *
 */
class Palette : public cello::Object
{
    /**
     * @brief Validator function for color references. Checks if the color is valid,
     * and if not, look it up in the palette by name.
     *
     * @param value The color to validate.
     * @return The validated color, or the invalid color if the color is not valid.
     */
    cello::Value<PaletteColor>::ValidateGetFn checkReference = [this] (const PaletteColor& value)
    {
        if (static_cast<juce::Colour> (value) != PaletteColor::invalidColor)
            return value;

        return this->getattr (value.toString (), PaletteColor {});
    };

public:
    static const inline juce::Identifier type { "Palette" };

    Palette (const cello::Object& parentOrSelf)
    : cello::Object { type.toString (), parentOrSelf }
    {
    }

    MAKE_VALUE_MEMBER (juce::String, name, "default (light)");
    MAKE_VALUE_MEMBER (juce::Font, monospaceFont,
                       juce::Font (juce::FontOptions { "Fira Code", 15.f, juce::Font::FontStyleFlags::plain }));

    // colors from the juce ColourScheme struct:
    COLOR_M (windowBackground, "#efefef");
    COLOR_M (widgetBackground, "#fff");
    COLOR_M (menuBackground, "#fff");
    COLOR_M (outline, "#ddd");
    COLOR_M (defaultText, "#000");
    COLOR_M (defaultFill, "#a9a9a9");
    COLOR_M (highlightedText, "#fff");
    COLOR_M (highlightedFill, "#42a2c8");
    COLOR_M (menuText, "#000");
    COLOR_M (umpBackground, "#eee");
    COLOR_M (midi2Label, "#555");
    COLOR_M (midi2Value, "Green");

private:
};
