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
    COLOR_M (windowBackground, "#060e20");
    COLOR_M (widgetBackground, "#fff");
    COLOR_M (menuBackground, "#fff");
    COLOR_M (outline, "#2d3449");
    COLOR_M (defaultText, "#dae2fe");
    COLOR_M (secondaryText, "0x94a3b8");
    COLOR_M (defaultFill, "#a9a9a9");
    COLOR_M (highlightedText, "#fff");
    COLOR_M (highlightedFill, "#42a2c8");
    COLOR_M (menuText, "#000");
    COLOR_M (surfaceMid, "#0b1426");
    COLOR_M (surfaceHigh, "#1a2332");
    COLOR_M (divider, "outline");
    COLOR_M (deepBackground, "#060e20");
    COLOR_M (umpBackground, "deepBackground");

    // MIDI 2.0 (UMP) — Emerald 400 / Emerald 800
    COLOR_M (midi2Label, "#065F46");
    COLOR_M (midi2Value, "#4EDE91");

    // MIDI 1.0 (Legacy) — Amber 500 / Amber 900
    COLOR_M (midi1Label, "#78350F");
    COLOR_M (midi1Value, "#FFB000");

    // Utility — Slate 400 / Slate 700
    COLOR_M (utilityLabel, "#334155");
    COLOR_M (utilityValue, "#94A3B8");

    // Common/Realtime — Rose 500 / Rose 900
    COLOR_M (commonRealtimeLabel, "#881337");
    COLOR_M (commonRealtimeValue, "#F43F5E");

    // 7-bit SysEx — Purple 500 / Purple 900
    COLOR_M (sysex7Label, "#581C87");
    COLOR_M (sysex7Value, "#A855F7");

    // 8-bit SysEx — Blue 500 / Blue 900
    COLOR_M (sysex8Label, "#1E3A8A");
    COLOR_M (sysex8Value, "#3B82F6");

    // Flex Data — Cyan 500 / Cyan 900
    COLOR_M (flexDataLabel, "#164E63");
    COLOR_M (flexDataValue, "#06B6D4");

    // Stream Data — Yellow 500 / Yellow 900
    COLOR_M (streamLabel, "#713F12");
    COLOR_M (streamValue, "#EAB308");

    // Undefined/Error — Red 500 / Red 900
    COLOR_M (undefinedLabel, "#7F1D1D");
    COLOR_M (undefinedValue, "#EF4444");

    // Endpoint status
    COLOR_M (endpointNotAlive, "surfaceHigh");
    COLOR_M (endpointInputAlive, "#166534");
    COLOR_M (endpointOutputAlive, "#7F1D1D");

    // Resize handle active (hover/drag)
    COLOR_M (resizeHandleActive, "outline");

private:
};
