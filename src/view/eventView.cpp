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
#include "palette.h"

EventView::EventView (AppContext& theAppContext)
: appContext { theAppContext }
{
}

void EventView::paint (juce::Graphics& g)
{
    Palette palette { PersistentContext { appContext } };
    g.fillAll (palette.windowBackground.get ());

    const float h = static_cast<float> (getHeight ());

    // column divider lines
    g.setColour (juce::Colours::darkgrey);
    g.drawVerticalLine (kDivider1X, 0.0f, h);
    g.drawVerticalLine (kDivider2X, 0.0f, h);

    // bottom border between events
    g.setColour (juce::Colours::grey);
    g.drawHorizontalLine (getHeight () - 1, 0.0f, static_cast<float> (getWidth ()));
}

void EventView::resized ()
{
    if (timeValue)
        timeValue->setBounds (0, 0, kTimeColumnEnd, kRowHeight);

    if (endpointValue)
        endpointValue->setBounds (kDivider1X + 2, 0,
                                  kEndpointColumnEnd - kDivider1X - 2, kRowHeight);

    if (dataValues.empty ())
        return;

    const int availableWidth = getWidth () - kValuesOriginX;
    const bool shouldWrap    = getWidth () >= kMinWrapWidth;

    int currentX = kValuesOriginX;
    int currentY = 0;

    for (auto& val : dataValues)
    {
        const int valWidth = static_cast<int> (val->getWidth ()) + kValueGap;

        if (shouldWrap && currentX > kValuesOriginX && (currentX - kValuesOriginX + valWidth) > availableWidth)
        {
            currentX = kValuesOriginX;
            currentY += kRowHeight;
        }

        val->setBounds (currentX, currentY, static_cast<int> (val->getWidth ()), kRowHeight);
        currentX += valWidth;
    }
}

void EventView::reset ()
{
    if (timeValue)
    {
        removeChildComponent (timeValue.get ());
        timeValue.reset ();
    }

    if (endpointValue)
    {
        removeChildComponent (endpointValue.get ());
        endpointValue.reset ();
    }

    for (auto& val : dataValues)
        removeChildComponent (val.get ());
    dataValues.clear ();

    // Reset size to zero so that the next sizeToWidth() call always triggers
    // resized(), which positions the newly-created child components.
    setSize (0, 0);
}

void EventView::setTime (const juce::String& value)
{
    if (timeValue)
        removeChildComponent (timeValue.get ());
    timeValue = std::make_unique<LabeledValue> (appContext, "", value);
    addAndMakeVisible (*timeValue);
}

void EventView::setEndpoint (const juce::String& value)
{
    if (endpointValue)
        removeChildComponent (endpointValue.get ());
    endpointValue = std::make_unique<LabeledValue> (appContext, "", value);
    addAndMakeVisible (*endpointValue);
}

void EventView::addValue (const juce::String& label, const juce::String& value)
{
    auto lv = std::make_unique<LabeledValue> (appContext, label, value);
    addAndMakeVisible (*lv);
    dataValues.push_back (std::move (lv));
}

int EventView::getContentHeight (int width) const
{
    if (dataValues.empty ())
        return kRowHeight;

    if (width < kMinWrapWidth)
        return kRowHeight;

    const int availableWidth = width - kValuesOriginX;
    int currentX             = 0;
    int numRows              = 1;

    for (const auto& val : dataValues)
    {
        const int valWidth = static_cast<int> (val->getWidth ()) + kValueGap;
        if (currentX > 0 && currentX + valWidth > availableWidth)
        {
            ++numRows;
            currentX = 0;
        }
        currentX += valWidth;
    }

    return numRows * kRowHeight;
}
