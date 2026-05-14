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
, header { appContext }
, eventListView { appContext }
, settingsView { appContext }
, col1Resizer { runtimeContext.col1Width, persistentContext.eventViewContext.col1Width, persistentContext.dragging }
, col2Resizer { runtimeContext.col2Width, persistentContext.eventViewContext.col2Width, persistentContext.dragging }
, col3Resizer { runtimeContext.col3Width, persistentContext.eventViewContext.col3Width, persistentContext.dragging }
{
    // Z-order: viewport at back, header above it, resizers above header (full-height
    // drag), settingsViewport at front so it renders over the resizers when open.
    addAndMakeVisible (viewport);
    addAndMakeVisible (header);
    col1Resizer.setHeaderHeight (DataViewHeader::kHeaderHeight);
    col2Resizer.setHeaderHeight (DataViewHeader::kHeaderHeight);
    col3Resizer.setHeaderHeight (DataViewHeader::kHeaderHeight);
    addAndMakeVisible (col1Resizer);
    addAndMakeVisible (col2Resizer);
    addAndMakeVisible (col3Resizer);
    addAndMakeVisible (settingsViewport);

    viewport.setViewedComponent (&eventListView, false);
    settingsViewport.setViewedComponent (&settingsView, false);
    settingsViewport.setScrollBarsShown (true, false);

    viewport.getVerticalScrollBar ().addListener (this);
    viewport.addMouseListener (this, true);

    runtimeContext.col1Width.onPropertyChange ([this] (const juce::Identifier&) { repaint (); resized (); });
    runtimeContext.col2Width.onPropertyChange ([this] (const juce::Identifier&) { repaint (); resized (); });
    runtimeContext.col3Width.onPropertyChange ([this] (const juce::Identifier&) { repaint (); resized (); });
    runtimeContext.settingsPos.onPropertyChange ([this] (const juce::Identifier&) { resized (); });
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
    const float top             = static_cast<float> (DataViewHeader::kHeaderHeight);
    const float bottom          = static_cast<float> (getHeight ());

    g.setColour (palette.outline.get ());
    g.drawVerticalLine (line1X, top, bottom);
    g.drawVerticalLine (line2X, top, bottom);
    g.drawVerticalLine (line3X, top, bottom);
}

void DataView::resized ()
{
    auto bounds = getLocalBounds ();
    header.setBounds (bounds.removeFromTop (DataViewHeader::kHeaderHeight));
    viewport.setBounds (bounds);
    // Use the viewport's visible content width rather than the full DataView
    // width: when the vertical scrollbar is visible it occupies some pixels
    // on the right, so the content area is narrower than getWidth().
    eventListView.widthChanged (viewport.getMaximumVisibleWidth ());
    eventListView.visibleAreaChanged (viewport.getViewArea ());

    // Position resizers: full height of DataView, centred on each column boundary.
    constexpr int kDividerWidth = 5;
    const int col1W             = runtimeContext.col1Width.get ();
    const int col2W             = runtimeContext.col2Width.get ();
    const int col3W             = runtimeContext.col3Width.get ();
    const int half              = kDividerWidth / 2;
    const int h                 = getHeight ();
    col1Resizer.setBounds (col1W - half, 0, kDividerWidth, h);
    col2Resizer.setBounds (col1W + kDividerWidth + col2W - half, 0, kDividerWidth, h);
    col3Resizer.setBounds (col1W + kDividerWidth + col2W + kDividerWidth + col3W - half, 0, kDividerWidth, h);

    // Position the settings panel: centered horizontally, sliding down from
    // above the view. At settingsPos=0 it is entirely above y=0 (invisible).
    // At settingsPos=1 its top edge is flush with the bottom of the header.
    // The settingsViewport caps the visible height to the available space so the
    // panel never overflows the window; the content scrolls inside it.
    const float settingsPos = runtimeContext.settingsPos;
    const int settingsW     = settingsView.getWidth ();
    const int availableH    = getHeight () - DataViewHeader::kHeaderHeight;
    const int settingsH     = juce::jmin (settingsView.getHeight (), availableH);
    const int settingsX     = (getWidth () - settingsW) / 2;
    const int settingsY     = juce::roundToInt ((settingsH + DataViewHeader::kHeaderHeight) * settingsPos) - settingsH;
    settingsViewport.setBounds (settingsX, settingsY, settingsW, settingsH);
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
