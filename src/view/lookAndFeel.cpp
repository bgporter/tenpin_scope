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

#include "lookAndFeel.h"

_10PinLookAndFeel::_10PinLookAndFeel (const AppContext& appContext)
: persistentContext { appContext }
, palette { persistentContext }
{
    // initialize the built-in colors from the palette
    updateFromPalette ();
    palette.onPropertyChange ([this] (const juce::Identifier& /*id*/) { updateFromPalette (); });

    // for now, fonts are hardcoded.
    updateFonts ();
}

void _10PinLookAndFeel::updateFromPalette ()
{
    setColourScheme ({ palette.windowBackground.get (), palette.widgetBackground.get (), palette.menuBackground.get (),
                       palette.outline.get (), palette.defaultText.get (), palette.defaultFill.get (),
                       palette.highlightedText.get (), palette.highlightedFill.get (), palette.menuText.get () });

    setColour (juce::ComboBox::backgroundColourId, palette.widgetBackground.get ());
    setColour (juce::ComboBox::textColourId, palette.menuText.get ());
    setColour (juce::ToggleButton::textColourId, palette.menuText.get ());
    setColour (juce::ToggleButton::tickColourId, palette.menuText.get ());
}

void _10PinLookAndFeel::updateFonts ()
{
    // Labels use IBM Plex Sans at its Condensed/Bold corner (wdth range is [75, 100], with
    // 75 == Condensed; wght range is [100, 700], with 700 == Bold), matching the static
    // IBMPlexSans_Condensed-Bold.ttf this replaced.
    auto labelBase = juce::Typeface::createSystemTypefaceFor (FontData::IBMPlexSansVariableFont_wdthwght_ttf,
                                                              FontData::IBMPlexSansVariableFont_wdthwght_ttfSize);
    labelTypeface = labelBase->cloneWithVariableSettings (
        std::array { juce::FontVariableSetting ("wdth", 75.f), juce::FontVariableSetting ("wght", 700.f) });

    // Values use Fira Code at Bold (wght == 700), matching the static FiraCode-Bold.ttf this replaced.
    auto valueBase =
        juce::Typeface::createSystemTypefaceFor (FontData::FiraCodeVariableFont_wght_ttf, FontData::FiraCodeVariableFont_wght_ttfSize);
    valueTypeface = valueBase->cloneWithVariableSettings (std::array { juce::FontVariableSetting ("wght", 700.f) });
}

juce::Typeface::Ptr _10PinLookAndFeel::getTypefaceForFont (const juce::Font& f)
{
    if (f.getTypefaceName ().containsIgnoreCase ("Monospace"))
        return valueTypeface;
    return labelTypeface;
}

juce::Font _10PinLookAndFeel::getLabelFont () const
{
    juce::FontOptions options (labelTypeface);
    return juce::Font (options.withHeight (persistentContext.eventViewContext.textHeight));
}

juce::Font _10PinLookAndFeel::getValueFont () const
{
    juce::FontOptions options (valueTypeface);
    return juce::Font (options.withHeight (persistentContext.eventViewContext.textHeight));
}
