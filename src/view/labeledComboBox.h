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

/**
 * @brief A label + combo box pair laid out in a single row.
 *
 * The label occupies a fixed-width strip on the left; the combo box fills
 * the remainder. Font and colour are sourced from TenpinLookAndFeel and
 * updated automatically when the look and feel changes.
 *
 * comboBox is public so callers can populate items and wire up callbacks
 * without this class needing to know about the app context.
 */
class LabeledComboBox : public juce::Component
{
public:
    LabeledComboBox (juce::StringRef labelText, int labelWidth = 120);

    void resized () override;
    void lookAndFeelChanged () override;

    juce::ComboBox comboBox;

private:
    static constexpr int kGap { 4 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LabeledComboBox)
    juce::Label label;
    int labelWidth;
};
