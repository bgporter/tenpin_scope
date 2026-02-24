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

#include "model/ump/umpEvent.h"
#include "palette.h"
#include "utility/logger.h"

EventListView::EventListView (AppContext& theAppContext, OnEventListChangedFn onEventListChangedFn,
                              OnBeforeEventAddedFn onBeforeEventAddedFn)
: appContext { theAppContext }
, runtimeContext { appContext }
, midiProperties { runtimeContext }
, eventList { midiProperties.midiEvents }
, onEventListChanged { std::move (onEventListChangedFn) }
, onBeforeEventAdded { std::move (onBeforeEventAddedFn) }
, handler { std::make_unique<EventListViewHandler> (appContext) }
{
    eventList.onChildAdded      = [this] (juce::ValueTree& vt, int, int) { handleEventAdded (vt); };
    eventList.onChildrenCleared = [this] () { handleChildrenCleared (); };

    // Sync initial state if list already has events
    const int count = eventList.getNumChildren ();
    if (count > 0)
    {
        for (int i = 0; i < count; ++i)
        {
            UmpEvent event (eventList[i]);
            if (handler->handle (event) == UmpHandler::Result::ok)
            {
                auto view           = handler->getEventView ();
                const int width     = getWidth ();
                const int newHeight = view->getContentHeight (width);
                eventHeights.push_back (newHeight);
                int prevCumulative = cumulativeHeights.empty () ? 0 : cumulativeHeights.back ();
                cumulativeHeights.push_back (prevCumulative + newHeight);
            }
        }
    }
}

void EventListView::handleEventAdded (juce::ValueTree& vt)
{
    if (onBeforeEventAdded)
        onBeforeEventAdded ();

    UmpEvent event (vt);
    if (handler->handle (event) != UmpHandler::Result::ok)
        return;

    auto eventView      = handler->getEventView ();
    const int width     = getWidth ();
    const int newHeight = eventView->getContentHeight (width);
    eventView->setSize (width, newHeight);

    const int eventIndex = static_cast<int> (eventHeights.size ());

    eventHeights.push_back (newHeight);
    int prevCumulative = cumulativeHeights.empty () ? 0 : cumulativeHeights.back ();
    cumulativeHeights.push_back (prevCumulative + newHeight);

    // After a clear, currentVisibleRange is empty; establish it so new events get views
    if (currentVisibleRange.isEmpty ())
        currentVisibleRange = juce::Range<int> (eventIndex, eventIndex + 1);

    if (isInBufferZone (eventIndex))
    {
        if (newHeight > 0)
            addAndMakeVisible (eventView.get ());
        activeViews[eventIndex] = std::move (eventView);
        positionView (eventIndex);
    }

    updateContentSize ();

    if (onEventListChanged)
        onEventListChanged (EventListChange::appended, eventList.getNumChildren ());
}

void EventListView::handleChildrenCleared ()
{
    // for (auto& [index, view] : activeViews)
    //     removeChildComponent (view.get ());
    activeViews.clear ();
    eventHeights.clear ();
    cumulativeHeights.clear ();
    currentVisibleRange = juce::Range<int> ();

    // const int count = eventList.getNumChildren ();
    // if (count > 0)
    // {
    //     for (int i = 0; i < count; ++i)
    //     {
    //         UmpEvent event (eventList[i]);
    //         if (handler->handle (event) == UmpHandler::Result::ok)
    //         {
    //             auto view           = handler->getEventView ();
    //             const int width     = getWidth ();
    //             const int newHeight = view->getContentHeight (width);
    //             eventHeights.push_back (newHeight);
    //             int prevCumulative = cumulativeHeights.empty () ? 0 : cumulativeHeights.back ();
    //             cumulativeHeights.push_back (prevCumulative + newHeight);
    //         }
    //     }
    // }

    updateContentSize ();

    if (onEventListChanged)
        onEventListChanged (EventListChange::cleared, 0);
}

void EventListView::paint (juce::Graphics& g)
{
    Palette palette { PersistentContext { appContext } };
    g.fillAll (palette.windowBackground.get ());

    auto bounds = getLocalBounds ();
    g.setColour (juce::Colours::grey);
    g.drawHorizontalLine (bounds.getBottom () - 1, bounds.getX (), bounds.getRight ());
}

void EventListView::resized ()
{
    const int width = getWidth ();
    for (auto& [index, view] : activeViews)
    {
        int oldHeight = eventHeights[index];
        int newHeight = view->getContentHeight (width);

        if (newHeight != oldHeight)
        {
            eventHeights[index] = newHeight;
            recalculateCumulativeHeightsFrom (index);
        }

        positionView (index);
    }

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
    if (eventHeights.empty ())
        return;

    int firstVisible = findEventAtYPosition (newVisibleArea.getY ());
    int lastVisible  = findEventAtYPosition (newVisibleArea.getBottom ());

    int bufferStart = std::max (0, firstVisible - bufferZoneEvents);
    int bufferEnd   = std::min (static_cast<int> (eventHeights.size ()) - 1, lastVisible + bufferZoneEvents);

    juce::Range<int> newBufferRange (bufferStart, bufferEnd + 1);

    if (newBufferRange == currentVisibleRange)
        return;

    cullViewsOutsideRange (newBufferRange);
    createViewsInRange (newBufferRange);

    currentVisibleRange = newBufferRange;
}

int EventListView::findEventAtYPosition (int yPos) const
{
    if (cumulativeHeights.empty ())
        return 0;

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

    for (int index : toRemove)
    {
        removeChildComponent (activeViews[index].get ());
        activeViews.erase (index);
    }
}

void EventListView::createViewsInRange (const juce::Range<int>& range)
{
    const int modelSize = eventList.getNumChildren ();

    for (int i = range.getStart (); i < range.getEnd (); ++i)
    {
        if (i >= modelSize || activeViews.count (i) > 0)
            continue;

        UmpEvent event (eventList[i]);
        if (handler->handle (event) != UmpHandler::Result::ok)
            continue;

        auto view        = handler->getEventView ();
        const int width  = getWidth ();
        const int height = view->getContentHeight (width);
        view->setSize (width, height);
        if (i < static_cast<int> (eventHeights.size ()) && eventHeights[i] > 0)
            addAndMakeVisible (view.get ());
        activeViews[i] = std::move (view);
        positionView (i);
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

    if (newWidth != getWidth () || newHeight != getHeight ())
        setSize (newWidth, newHeight);
}

bool EventListView::isInBufferZone (int eventIndex) const
{
    if (currentVisibleRange.isEmpty ())
        return false;

    return currentVisibleRange.contains (eventIndex);
}
