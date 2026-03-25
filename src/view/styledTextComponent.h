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

/**
 * @brief A component that displays styled text using juce::AttributedString and juce::TextLayout.
 * Supports configurable margins and padding for text placement.
 */
class StyledTextComponent : public juce::Component
{
public:
    /**
     * @brief Construct a StyledTextComponent
     *
     * @param text The AttributedString to display
     * @param marginLeft Left margin in pixels
     * @param marginRight Right margin in pixels
     * @param marginTop Top margin in pixels
     * @param marginBottom Bottom margin in pixels
     */
    StyledTextComponent (const juce::AttributedString& text = juce::AttributedString {}, int marginLeft = 2,
                         int marginRight = 2, int marginTop = 2, int marginBottom = 2);

    /**
     * @brief Set the text to display
     *
     * @param text The new AttributedString to display
     */
    void setText (const juce::AttributedString& text);

    /**
     * @brief Get the attributed text
     *
     * @return const juce::AttributedString& The current AttributedString
     */
    const juce::AttributedString& getText () const { return attributedText; }

    /**
     * @brief Set the margins for the text layout
     *
     * @param left Left margin in pixels
     * @param right Right margin in pixels
     * @param top Top margin in pixels
     * @param bottom Bottom margin in pixels
     */
    void setMargins (int left, int right, int top, int bottom);

    /**
     * @brief Calculate the height needed to display the text at the given width
     *
     * @param availableWidth The width available for the text
     * @param minWidth The minimum width below which text will clip instead of wrapping
     * @return int The required height in pixels
     */
    int getRequiredHeight (int availableWidth, int minWidth = 50) const;

    void paint (juce::Graphics& g) override;
    void resized () override;

private:
    void updateTextLayout ();

    juce::AttributedString attributedText;
    juce::TextLayout textLayout;
    int marginLeft { 2 };
    int marginRight { 2 };
    int marginTop { 2 };
    int marginBottom { 2 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StyledTextComponent)
};
