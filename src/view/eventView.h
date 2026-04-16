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
#include <vector>

#include "labeledValue.h"
#include "model/appContext.h"

class EventView : public juce::Component
{
public:
    EventView (AppContext& theAppContext);

    void paint (juce::Graphics& g) override;
    void resized () override;

    /**
     * @brief Return this view to a clean state so it can be reused from the pool.
     */
    void reset ();

    void setColors (juce::Colour bg, juce::Colour label, juce::Colour value, juce::Colour outline)
    {
        bgColor       = bg;
        labelColour   = label;
        valueColour   = value;
        outlineColour = outline;
    }

    /**
     * @brief Set the event timestamp display (no label, value only).
     */
    void setTime (const juce::String& value);

    /**
     * @brief Set the endpoint name display (no label, value only).
     */
    void setEndpoint (const juce::String& value);

    /**
     * @brief Set the event type name display.
     */
    void setEvent (const juce::String& value);

    /**
     * @brief Append a (label, value) pair to the event data area.
     */
    void addValue (const juce::String& label, const juce::String& value);

    /**
     * @brief add an empty LabeledValue, which we'll interpret in our layout
     * algorithm as being a newline indicator.
     *
     */
    void addLine () { addValue ("", ""); }

    void sizeToWidth (int width) { setSize (width, getContentHeight (width)); }

    /**
     * @brief Re-run layout and repaint without changing bounds. Called during
     * column drag to reposition child components against updated column widths
     * while leaving the event's Y position and height stable.
     */
    void refreshLayout ()
    {
        resized ();
        repaint ();
    }

    /**
     * @brief Calculate the height this view needs at the given width.
     *
     * When width >= kMinWrapWidth, data values wrap onto additional rows as
     * needed, growing the height. Below kMinWrapWidth a single row is used
     * and values that overflow clip off-screen.
     */
    int getContentHeight (int width) const;

private:
    // ---------- column layout constants ----------
    /// Width of each inter-column ResizeHandle, matching DataViewHeader::kDividerWidth.
    static constexpr int kDividerWidth = 5;
    static constexpr int kMinWrapWidth = 500; ///< below this width values stop wrapping
    /// Fraction of the natural text height added as padding on each side (top, bottom, left, right).
    static constexpr float kPaddingFraction = 0.25f;

    /** @brief Compute the pixel height of one row from the timestamp label's natural height. */
    int getRowHeight () const;

    AppContext appContext;
    std::unique_ptr<LabeledValue> timeValue;
    std::unique_ptr<LabeledValue> endpointValue;
    std::unique_ptr<LabeledValue> eventValue;
    std::vector<std::unique_ptr<LabeledValue>> dataValues;

    // colors
    juce::Colour bgColor;
    juce::Colour labelColour;
    juce::Colour valueColour;
    juce::Colour outlineColour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EventView)
};
