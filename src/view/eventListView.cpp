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

#include "eventListView.h"

#include "palette.h"
#include "utility/logger.h"

EventListView::EventListView (AppContext& theAppContext)
: appContext { theAppContext }
{
}

void EventListView::paint (juce::Graphics& g)
{
    Palette palette { PersistentContext { appContext } };
    // g.fillAll (palette.windowBackground.get ());
    g.fillAll (juce::Colours::red);

    // Temporary: draw a black X from corner to corner
    auto bounds = getLocalBounds ();
    g.setColour (juce::Colours::black);
    g.drawLine (bounds.getX (), bounds.getY (), bounds.getRight (), bounds.getBottom (), 1.0f);
    g.drawLine (bounds.getRight (), bounds.getY (), bounds.getX (), bounds.getBottom (), 1.0f);
}

void EventListView::resized ()
{
    // Width change requires remeasuring and repositioning active views
    for (auto& [index, view] : activeViews)
    {
        int oldHeight = eventHeights[index];
        int newHeight = view->getContentHeight (); // Remeasure with new width

        if (newHeight != oldHeight)
        {
            eventHeights[index] = newHeight;
            // Recalculate cumulative heights from this point forward
            recalculateCumulativeHeightsFrom (index);
        }

        positionView (index);
    }

    updateContentSize ();
}

void EventListView::addEventView (std::unique_ptr<EventView> eventView)
{
    const int eventIndex = static_cast<int> (eventHeights.size ());
    const int newHeight  = eventView->getContentHeight ();

    // Store metadata (always)
    eventHeights.push_back (newHeight);
    eventData.push_back (eventView->getDescription ());

    // Update cumulative heights
    int prevCumulative = cumulativeHeights.empty () ? 0 : cumulativeHeights.back ();
    cumulativeHeights.push_back (prevCumulative + newHeight);

    // Only create view if in buffer zone
    if (isInBufferZone (eventIndex))
    {
        if (newHeight > 0)
            addAndMakeVisible (eventView.get ());
        activeViews[eventIndex] = std::move (eventView);
        positionView (eventIndex);
    }
    // Otherwise just discard the view (metadata is saved)

    updateContentSize ();
}

int EventListView::getContentHeight () const
{
    return cumulativeHeights.empty () ? 0 : cumulativeHeights.back ();
}
void EventListView::parentSizeChanged ()
{
    const auto newWidth { getParentWidth () };
    const auto newHeight { std::max (getContentHeight (), getParentHeight ()) };

    if (newWidth != getWidth () || newHeight != getHeight ())
        setSize (newWidth, newHeight);
    else
        resized ();
}

void EventListView::visibleAreaChanged (const juce::Rectangle<int>& newVisibleArea)
{
    // Early exit if no events
    if (eventHeights.empty ())
        return;

    // Calculate which event indices are visible
    int firstVisible = findEventAtYPosition (newVisibleArea.getY ());
    int lastVisible  = findEventAtYPosition (newVisibleArea.getBottom ());

    // Expand to buffer zone
    int bufferStart = std::max (0, firstVisible - bufferZoneEvents);
    int bufferEnd   = std::min (static_cast<int> (eventHeights.size ()) - 1, lastVisible + bufferZoneEvents);

    juce::Range<int> newBufferRange (bufferStart, bufferEnd + 1);

    // Skip if range hasn't changed
    if (newBufferRange == currentVisibleRange)
        return;

    // Cull views outside buffer
    cullViewsOutsideRange (newBufferRange);

    // Create views inside buffer that don't exist
    createViewsInRange (newBufferRange);

    currentVisibleRange = newBufferRange;
}

int EventListView::findEventAtYPosition (int yPos) const
{
    if (cumulativeHeights.empty ())
        return 0;

    // Binary search in cumulativeHeights for O(log N) lookup
    auto it = std::upper_bound (cumulativeHeights.begin (), cumulativeHeights.end (), yPos);
    if (it == cumulativeHeights.end ())
        return static_cast<int> (eventHeights.size ()) - 1;

    return static_cast<int> (std::distance (cumulativeHeights.begin (), it));
}

void EventListView::cullViewsOutsideRange (const juce::Range<int>& keepRange)
{
    std::vector<int> toRemove;

    for (const auto& [index, view] : activeViews)
    {
        if (!keepRange.contains (index))
            toRemove.push_back (index);
    }

    if (!toRemove.empty ())
    {
        TRACE_ ({
            {              "msg",                                             "Culling views outside buffer zone"},
            {            "count",                                             static_cast<int> (toRemove.size ())},
            {"activeViewsBefore",                                          static_cast<int> (activeViews.size ())},
            {        "keepRange", juce::String (keepRange.getStart ()) + "-" + juce::String (keepRange.getEnd ())}
        });
    }

    for (int index : toRemove)
    {
        removeChildComponent (activeViews[index].get ());
        activeViews.erase (index);
    }
}

void EventListView::createViewsInRange (const juce::Range<int>& range)
{
    int createdCount = 0;

    for (int i = range.getStart (); i < range.getEnd (); ++i)
    {
        if (i >= static_cast<int> (eventData.size ()) || activeViews.count (i) > 0)
            continue;

        // Recreate view from stored data
        auto view = std::make_unique<EventView> (appContext, eventData[i]);
        if (eventHeights[i] > 0)
            addAndMakeVisible (view.get ());
        activeViews[i] = std::move (view);
        positionView (i);
        ++createdCount;
    }

    if (createdCount > 0)
    {
        TRACE_ ({
            {             "msg",                                         "Created views for buffer zone"},
            {           "count",                                                            createdCount},
            {"activeViewsAfter",                                  static_cast<int> (activeViews.size ())},
            {           "range", juce::String (range.getStart ()) + "-" + juce::String (range.getEnd ())},
            {     "totalEvents",                                    static_cast<int> (eventData.size ())}
        });
    }
}

void EventListView::positionView (int index)
{
    if (activeViews.count (index) == 0)
        return;

    int yPos   = (index == 0) ? 0 : cumulativeHeights[index - 1];
    int height = eventHeights[index];

    activeViews[index]->setBounds (0, yPos, getWidth (), height);
}

void EventListView::recalculateCumulativeHeightsFrom (int startIndex)
{
    if (startIndex >= static_cast<int> (eventHeights.size ()))
        return;

    int cumulative = (startIndex == 0) ? 0 : cumulativeHeights[startIndex - 1];

    for (int i = startIndex; i < static_cast<int> (eventHeights.size ()); ++i)
    {
        cumulative += eventHeights[i];
        cumulativeHeights[i] = cumulative;
    }
}

void EventListView::updateContentSize ()
{
    const auto newWidth { getParentWidth () };
    const auto newHeight { std::max (getContentHeight (), getParentHeight ()) };

    // Only update if size actually changed to avoid infinite loop
    if (newWidth != getWidth () || newHeight != getHeight ())
        setSize (newWidth, newHeight);
}

bool EventListView::isInBufferZone (int eventIndex) const
{
    // For newly added events, we're always at the end, so check if we're in the current visible range
    // If no visible range set yet, DON'T create views (will be created on first scroll update)
    if (currentVisibleRange.isEmpty ())
        return false;

    return currentVisibleRange.contains (eventIndex);
}
