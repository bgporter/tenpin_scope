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

#include "dataView.h"

#include "palette.h"

DataView::DataView (AppContext& theAppContext)
: appContext { theAppContext }
, persistentContext { appContext }
, runtimeContext { appContext }
, eventListView { appContext }
{
    addAndMakeVisible (viewport);

    viewport.setViewedComponent (&eventListView, false);

    viewport.getVerticalScrollBar ().addListener (this);
    viewport.addMouseListener (this, true);

    runtimeContext.col1Width.onPropertyChange ([this] (const juce::Identifier&) { repaint (); });
    runtimeContext.col2Width.onPropertyChange ([this] (const juce::Identifier&) { repaint (); });
    runtimeContext.col3Width.onPropertyChange ([this] (const juce::Identifier&) { repaint (); });
}

DataView::~DataView ()
{
    viewport.removeMouseListener (this);
    viewport.getVerticalScrollBar ().removeListener (this);
}

void DataView::paint (juce::Graphics& g)
{
    Palette palette { PersistentContext { appContext } };
    g.fillAll (palette.deepBackground.get ());

    constexpr int kDividerWidth = 5;
    const int col1Width         = runtimeContext.col1Width.get ();
    const int col2Width         = runtimeContext.col2Width.get ();
    const int col3Width         = runtimeContext.col3Width.get ();
    const int line1X            = col1Width;
    const int line2X            = col1Width + kDividerWidth + col2Width;
    const int line3X            = line2X + kDividerWidth + col3Width;
    g.setColour (palette.outline.get ());
    g.drawVerticalLine (line1X, 0.f, static_cast<float> (getHeight ()));
    g.drawVerticalLine (line2X, 0.f, static_cast<float> (getHeight ()));
    g.drawVerticalLine (line3X, 0.f, static_cast<float> (getHeight ()));
}

void DataView::resized ()
{
    auto bounds = getLocalBounds ();
    viewport.setBounds (bounds);
    // Use the viewport's visible content width rather than the full DataView
    // width: when the vertical scrollbar is visible it occupies some pixels
    // on the right, so the content area is narrower than getWidth().
    eventListView.widthChanged (viewport.getMaximumVisibleWidth ());
    eventListView.visibleAreaChanged (viewport.getViewArea ());
}

void DataView::scrollBarMoved (juce::ScrollBar* scrollBar, double newRangeStart)
{
    eventListView.visibleAreaChanged (viewport.getViewArea ());
}

void DataView::mouseDown (const juce::MouseEvent&)
{
    if (runtimeContext.settingsPos.get () > 0.f && onDismissSettings)
        onDismissSettings ();
}
