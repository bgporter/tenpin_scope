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

#include "eventView.h"

#include "model/persistentContext.h"
#include "model/runtimeContext.h"
#include "palette.h"

EventView::EventView (AppContext& theAppContext)
: appContext { theAppContext }
{
}

void EventView::paint (juce::Graphics& g)
{
    Palette palette { PersistentContext { appContext } };
    g.fillAll (bgColor.isTransparent () ? palette.umpBackground.get () : bgColor);

    RuntimeContext rc { appContext };
    const float h       = static_cast<float> (getHeight ());
    const int divider1X = rc.col1Width.get ();
    const int divider2X = divider1X + kDividerWidth + rc.col2Width.get ();
    const int divider3X = divider2X + kDividerWidth + rc.col3Width.get ();

    // column divider lines
    g.setColour (palette.outline.get ());
    g.drawVerticalLine (divider1X, 0.0f, h);
    g.drawVerticalLine (divider2X, 0.0f, h);
    g.drawVerticalLine (divider3X, 0.0f, h);

    // bottom border between events
    g.setColour (palette.outline.get ());
    g.drawHorizontalLine (getHeight () - 1, 0.0f, static_cast<float> (getWidth ()));
}

void EventView::resized ()
{
    if (!timeValue)
        return;

    RuntimeContext rc { appContext };
    const int col1Width     = rc.col1Width.get ();
    const int col2Width     = rc.col2Width.get ();
    const int col3Width     = rc.col3Width.get ();
    const int col2StartX    = col1Width + kDividerWidth;
    const int col3StartX    = col2StartX + col2Width + kDividerWidth;
    const int valuesOriginX = col3StartX + col3Width + kDividerWidth;

    const float naturalH = timeValue->getHeight ();
    const int padding    = static_cast<int> (naturalH * kPaddingFraction);
    const int contentH   = static_cast<int> (std::ceil (naturalH));
    const int rowH       = getRowHeight ();

    timeValue->setBounds (kDividerWidth, padding, col1Width, contentH);

    if (endpointValue)
        endpointValue->setBounds (col2StartX, padding, col2Width, contentH);

    if (eventValue)
        eventValue->setBounds (col3StartX, padding, col3Width, contentH);

    if (dataValues.empty ())
        return;

    const int availableWidth = getWidth () - valuesOriginX;
    const bool shouldWrap    = getWidth () >= kMinWrapWidth;

    int currentRow = 0;
    int currentX   = valuesOriginX;

    for (auto& val : dataValues)
    {
        const auto width { static_cast<int> (val->getWidth ()) };
        const auto isNewLine { width == 0 };
        const int valWidth { width + padding };

        if (isNewLine ||
            (shouldWrap && currentX > valuesOriginX && (currentX - valuesOriginX + valWidth) > availableWidth))
        {
            ++currentRow;
            currentX = valuesOriginX;
        }

        if (!isNewLine)
        {
            val->setBounds (currentX, currentRow * rowH + padding, width, contentH);
            currentX += valWidth;
        }
    }
}

void EventView::reset ()
{
    timeValue.reset ();
    endpointValue.reset ();
    eventValue.reset ();
    dataValues.clear ();
    // Reset size to zero so that the next sizeToWidth() call always triggers
    // resized(), which positions the newly-created child components.

    // temp for debugging: set all the colors to juce::Colours::transparentWhite
    bgColor       = juce::Colours::transparentWhite;
    labelColour   = juce::Colours::transparentWhite;
    valueColour   = juce::Colours::transparentWhite;
    outlineColour = juce::Colours::transparentWhite;
    setSize (0, 0);
}

void EventView::setTime (const juce::String& value)
{
    if (timeValue)
        removeChildComponent (timeValue.get ());
    timeValue = std::make_unique<LabeledValue> (appContext, "", labelColour, value, valueColour);
    addAndMakeVisible (*timeValue);
}

void EventView::setEndpoint (const juce::String& value)
{
    if (endpointValue)
        removeChildComponent (endpointValue.get ());
    endpointValue = std::make_unique<LabeledValue> (appContext, value, labelColour, "", valueColour);
    addAndMakeVisible (*endpointValue);
}

void EventView::setEvent (const juce::String& value)
{
    if (eventValue)
        removeChildComponent (eventValue.get ());
    eventValue = std::make_unique<LabeledValue> (appContext, value, labelColour, "", valueColour);
    addAndMakeVisible (*eventValue);
}

void EventView::addValue (const juce::String& label, const juce::String& value)
{
    auto lv = std::make_unique<LabeledValue> (appContext, label, labelColour, value, valueColour);
    addAndMakeVisible (*lv);
    dataValues.push_back (std::move (lv));
}

int EventView::getRowHeight () const
{
    if (timeValue)
    {
        const float naturalH = timeValue->getHeight ();
        return static_cast<int> (std::ceil (naturalH * (1.0f + 2.0f * kPaddingFraction)));
    }
    return 20;
}

int EventView::getContentHeight (int width) const
{
    const int rowH = getRowHeight ();

    if (dataValues.empty ())
        return rowH;

    if (width < kMinWrapWidth)
        return rowH;

    RuntimeContext rc { appContext };
    const int valuesOriginX =
        rc.col1Width.get () + kDividerWidth + rc.col2Width.get () + kDividerWidth + rc.col3Width.get () + kDividerWidth;
    const int availableWidth = width - valuesOriginX;
    const int padding        = timeValue ? static_cast<int> (timeValue->getHeight () * kPaddingFraction) : rowH / 4;
    int currentX             = 0;
    int numRows              = 1;

    for (const auto& val : dataValues)
    {
        const auto isNewLine = val->getWidth () == 0;
        const int valWidth   = static_cast<int> (val->getWidth ()) + padding;
        if (isNewLine || (currentX > 0 && (currentX + valWidth) > availableWidth))
        {
            ++numRows;
            currentX = 0;
        }
        currentX += valWidth;
    }

    return numRows * rowH;
}
