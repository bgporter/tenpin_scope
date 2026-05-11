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

#include "labeledValue.h"

#include "lookAndFeel.h"

namespace
{
// Hardcoded defaults — will be replaced with AppContext/PersistentContext lookups.
constexpr float kFontSize       = 13.0f;
constexpr float kVeryLargeWidth = 10000.0f;

const juce::Colour kLabelColour { 0xFF555555 };   // dark grey, legible on light background
const juce::Colour kValueColour { 0xFFEE1111 };   // near-black
const juce::Colour kOutlineColour { 0xFFCCCCCC }; // light grey, for debugging layout
} // namespace

LabeledValue::LabeledValue (AppContext& /* ctx */, juce::StringRef label, juce::Colour labelColor,
                            juce::StringRef value, juce::Colour valueColor)
{
    juce::AttributedString aString;
    auto& lnf = dynamic_cast<TenpinLookAndFeel&> (getLookAndFeel ());

    if (label.isNotEmpty ())
    {
        auto labelFont = lnf.getTenpinLabelFont ();
        aString.append (juce::String (label), labelFont, labelColor);
        // if we only have a label, do NOT append the colon.
        if (value.isNotEmpty ())
            aString.append (": ", labelFont, kLabelColour);
    }

    if (value.isNotEmpty ())
    {
        auto valueFont = lnf.getTenpinValueFont ();
        aString.append (juce::String (value), valueFont, valueColor);
    }

    text.createLayout (aString, kVeryLargeWidth);
}

void LabeledValue::paint (juce::Graphics& g)
{
    text.draw (g, getLocalBounds ().toFloat ());
    // g.setColour (kOutlineColour);
    // g.drawRect (getLocalBounds (), 1);
}
