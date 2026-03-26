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

#include "model/appContext.h"

/**
 * @brief A component that displays a (label, value) pair as styled inline text.
 *
 * Uses juce::AttributedString and juce::TextLayout to render a bold, muted label
 * followed by a bright value on a single line. Font and colour constants are
 * hardcoded for now and will be driven by AppContext/PersistentContext in a
 * future iteration.
 */
class LabeledValue : public juce::Component
{
public:
    /**
     * @brief Construct a LabeledValue.
     *
     * @param ctx       Application context (reserved for future font/colour lookup).
     * @param label     Short label string; if non-empty, rendered as "label: ".
     * @param value     Value string rendered after the label.
     */
    LabeledValue (AppContext& ctx, juce::StringRef label, juce::StringRef value);

    /** @return The natural rendered height of this component's text layout. */
    float getHeight () const { return text.getHeight (); }

    /** @return The natural rendered width of this component's text layout. */
    float getWidth () const { return text.getWidth (); }

    void paint (juce::Graphics& g) override;

private:
    juce::TextLayout text;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LabeledValue)
};
