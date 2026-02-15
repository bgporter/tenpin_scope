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

#include "styledTextComponent.h"

StyledTextComponent::StyledTextComponent (const juce::AttributedString& text, int marginLeft, int marginRight,
                                          int marginTop, int marginBottom)
: attributedText { text }
, marginLeft { marginLeft }
, marginRight { marginRight }
, marginTop { marginTop }
, marginBottom { marginBottom }
{
    updateTextLayout ();
}

void StyledTextComponent::setText (const juce::AttributedString& text)
{
    attributedText = text;
    updateTextLayout ();
    repaint ();
}

void StyledTextComponent::setMargins (int left, int right, int top, int bottom)
{
    marginLeft   = left;
    marginRight  = right;
    marginTop    = top;
    marginBottom = bottom;
    updateTextLayout ();
    repaint ();
}

int StyledTextComponent::getRequiredHeight (int availableWidth, int minWidth) const
{
    // If width is below minimum, return single-line height (text will clip)
    if (availableWidth < minWidth)
    {
        juce::TextLayout tempLayout;
        tempLayout.createLayout (attributedText, static_cast<float> (minWidth - marginLeft - marginRight));
        return static_cast<int> (tempLayout.getHeight ()) + marginTop + marginBottom;
    }

    // Calculate actual height needed for the given width
    juce::TextLayout tempLayout;
    float layoutWidth = static_cast<float> (availableWidth - marginLeft - marginRight);
    tempLayout.createLayout (attributedText, layoutWidth);
    return static_cast<int> (tempLayout.getHeight ()) + marginTop + marginBottom;
}

void StyledTextComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds ();
    bounds.removeFromLeft (marginLeft);
    bounds.removeFromRight (marginRight);
    bounds.removeFromTop (marginTop);
    bounds.removeFromBottom (marginBottom);

    // Set clip region to prevent text from drawing outside bounds
    g.saveState ();
    g.reduceClipRegion (bounds);

    // Draw the text layout
    textLayout.draw (g, bounds.toFloat ());

    g.restoreState ();
}

void StyledTextComponent::resized ()
{
    updateTextLayout ();
}

void StyledTextComponent::updateTextLayout ()
{
    auto bounds = getLocalBounds ();
    bounds.removeFromLeft (marginLeft);
    bounds.removeFromRight (marginRight);
    bounds.removeFromTop (marginTop);
    bounds.removeFromBottom (marginBottom);

    float layoutWidth = static_cast<float> (juce::jmax (1, bounds.getWidth ()));

    textLayout.createLayout (attributedText, layoutWidth);
}
