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

#include <algorithm>

#include "model/ump/umpEvent.h"
#include "palette.h"
#include "utility/logger.h"

EventListView::EventListView (AppContext& theAppContext)
: appContext { theAppContext }
, runtimeContext { appContext }
, persistentContext { appContext }
, midiProperties { runtimeContext }
, eventList { midiProperties.midiEvents }
, handler { std::make_unique<EventListViewHandler> (appContext) }
, eventViewPool { appContext }
{
    eventList.onChildAdded      = [this] (juce::ValueTree& vt, int, int) { addEvent (vt); };
    eventList.onChildrenCleared = [this] () { clear (); };
    eventList.isRebuilding.onPropertyChange (
        [this] (const juce::Identifier&)
        {
            if (!eventList.isRebuilding.get () && viewport != nullptr)
            {
                visibleArea = {};
                viewport->setViewPositionProportionately (0.0, 1.0);
            }
        });

    // Seed runtime context with persisted values so initial layout is correct.
    runtimeContext.col1Width = persistentContext.col1Width.get ();
    runtimeContext.col2Width = persistentContext.col2Width.get ();

    runtimeContext.col1Width.onPropertyChange ([this] (const juce::Identifier&) { refreshVisibleViews (); });
    runtimeContext.col2Width.onPropertyChange ([this] (const juce::Identifier&) { refreshVisibleViews (); });

    persistentContext.dragging.onPropertyChange (
        [this] (const juce::Identifier&)
        {
            if (!persistentContext.dragging.get ())
                forceRebuild ();
        });

    // Sync initial state if list already has events?
    // I'm not sure if this can happen, figure it out later.
    // iterate through the event list,
}

void EventListView::clear ()
{
    TRACE_ ("EventListView::clear");
    while (!visibleEventViews.empty ())
    {
        auto& eventView = visibleEventViews.front ();
        removeChildComponent (eventView.get ());
        eventViewPool.returnEventView (std::move (eventView));
        visibleEventViews.pop_front ();
    }
    visibleEventRange = juce::Range<int> ();
    eventPositions.clear ();
    setSize (getWidth (), 0);
}

std::unique_ptr<EventView> EventListView::createEventView (juce::ValueTree vt, int index, int width)
{
    UmpEvent event (vt);
    auto eventView { eventViewPool.getEventView () };
    const auto result { handler->handle (event, index, eventView.get (), width) };
    if (result != UmpHandler::Result::ok)
    {
        const auto fmt { juce::XmlElement::TextFormat ().singleLine ().withoutHeader () };
        WARN_ ({
            {   "msg",  "Failed to handle event"},
            { "event",      vt.toXmlString (fmt)},
            {"result", static_cast<int> (result)}
        });
        eventViewPool.returnEventView (std::move (eventView));
        return nullptr;
    }
    return eventView;
}

void EventListView::addEvent (juce::ValueTree vt)
{
    int index = static_cast<int> (eventList.getNumChildren ()) - 1;
    auto eventView { createEventView (vt, index, getWidth ()) };
    if (eventView == nullptr)
        return;

    // we always need to know where this new view should be positioned, and we
    // always need to grow the component to hold it, whether we are about to actually
    // display it or not.
    // When we add a new event, it always begins at the current height
    // of this view.

    const auto eventPosition = getHeight ();
    const auto eventHeight { eventView->getHeight () };
    eventPositions.push_back (eventPosition);

    // Always grow the content to include this event, even if we won't display it.
    setSize (getWidth (), eventPosition + eventHeight);

    if (!shouldDisplayNewEvent ())
    {
        eventViewPool.returnEventView (std::move (eventView));
        return;
    }

    // Decide whether the viewport was following the bottom before this event.
    // "At bottom" means the previous last event was visible; we check against
    // its top pixel rather than the exact content-bottom to avoid a permanent
    // failure from the small rounding/scrollbar discrepancy in
    // setViewPositionProportionately (observed ~8 px in practice).
    const auto viewportHeight = viewport->getMaximumVisibleHeight ();
    const auto viewPosY       = viewport->getViewPositionY ();
    const bool wasAtBottom    = (viewportHeight == 0)
                                || (index == 0)
                                || (viewPosY + viewportHeight > eventPositions[index - 1]);

    if (!wasAtBottom)
    {
        // New event is below the visible area; don't add it to the deque.
        // visibleAreaChanged will populate it when the user scrolls there.
        eventViewPool.returnEventView (std::move (eventView));
        return;
    }

    // We are following the bottom: display the new event, scroll to it, then
    // immediately trim any views that have scrolled off the top.  We call
    // visibleAreaChanged directly here rather than waiting for the async
    // scrollBarMoved notification -- without this, many events accumulate in
    // the deque between message-loop cycles before a single batch trim fires.
    displayEvent (std::move (eventView), index, InsertionPoint::bottom);
    viewport->setViewPositionProportionately (0.0, 1.0);
    visibleAreaChanged (viewport->getViewArea ());
}

void EventListView::displayEvent (std::unique_ptr<EventView> eventView, int index, InsertionPoint insertionPoint)
{
    if (eventView == nullptr)
    {
        ERROR_ ("EventListView::displayEvent: eventView is nullptr");
        return;
    }
    if (index < 0 || index >= static_cast<int> (eventPositions.size ()))
    {
        ERROR_ ("EventListView::displayEvent: index is out of bounds");
        return;
    }
    eventView->setTopLeftPosition (0, eventPositions[index]);
    addAndMakeVisible (eventView.get ());

    switch (insertionPoint)
    {
        case InsertionPoint::top:
            visibleEventViews.push_front (std::move (eventView));
            visibleEventRange.setStart (visibleEventRange.isEmpty ()
                                            ? index
                                            : std::min (visibleEventRange.getStart (), index));
            break;
        case InsertionPoint::bottom:
            visibleEventViews.push_back (std::move (eventView));
            if (visibleEventRange.isEmpty ())
                visibleEventRange.setStart (index);
            visibleEventRange.setEnd (index + 1);
            break;
    }
}

void EventListView::hideEvent (InsertionPoint insertionPoint)
{
    switch (insertionPoint)
    {
        case InsertionPoint::top:
        {
            auto& eventView = visibleEventViews.front ();
            removeChildComponent (eventView.get ());
            eventViewPool.returnEventView (std::move (eventView));
            visibleEventViews.pop_front ();
            visibleEventRange.setStart (visibleEventRange.getStart () + 1);
            break;
        }
        case InsertionPoint::bottom:
        {
            auto& eventView = visibleEventViews.back ();
            removeChildComponent (eventView.get ());
            eventViewPool.returnEventView (std::move (eventView));
            visibleEventViews.pop_back ();
            visibleEventRange.setEnd (visibleEventRange.getEnd () - 1);
            break;
        }
    }
}

void EventListView::widthChanged (int newWidth)
{
    if (newWidth == getWidth ())
        return;
    DBG ("EventListView::widthChanged: " << newWidth);
    setSize (newWidth, getHeight ()); // update width so forceRebuild() sees the new value
    if (persistentContext.dragging.get ())
    {
        for (auto& eventView : visibleEventViews)
            eventView->sizeToWidth (newWidth);
    }
    else
    {
        forceRebuild ();
    }
}

void EventListView::forceRebuild ()
{
    const int newWidth = getWidth ();
    DBG ("EventListView::forceRebuild at width: " << newWidth);

    isRecalculating = true;
    int newHeight { 0 };
    clear ();
    setSize (newWidth, 0);

    for (int i = 0; i < eventList.getNumChildren (); ++i)
    {
        eventPositions.push_back (newHeight);
        auto eventView { createEventView (eventList[i], i, newWidth) };
        if (eventView == nullptr)
            continue;
        newHeight += eventView->getHeight ();
        eventViewPool.returnEventView (std::move (eventView));
    }

    isRecalculating = false;
    visibleArea      = {}; // force visibleAreaChanged to re-evaluate after rebuild
    setSize (newWidth, newHeight);
}

void EventListView::refreshVisibleViews ()
{
    for (auto& eventView : visibleEventViews)
        eventView->refreshLayout ();
}

bool EventListView::shouldDisplayNewEvent () const
{
    return !eventList.isRebuilding.get ();
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
    // const int width = getWidth ();
    // for (auto& [index, view] : activeViews)
    // {
    //     int oldHeight = eventHeights[index];
    //     int newHeight = view->getContentHeight (width);

    //     if (newHeight != oldHeight)
    //     {
    //         eventHeights[index] = newHeight;
    //         recalculateCumulativeHeightsFrom (index);
    //     }

    //     positionView (index);
    // }

    // updateContentSize ();
}

void EventListView::visibleAreaChanged (const juce::Rectangle<int>& newVisibleArea)
{
    if (isRecalculating)
        return;

    if (visibleArea == newVisibleArea)
        return;

    visibleArea = newVisibleArea;

    DBG ("visibleAreaChanged: " << newVisibleArea.toString ());

    int firstVisible    = findEventAtYPosition (newVisibleArea.getY ());
    const auto newStart = std::max (firstVisible - 2, 0);
    int lastVisible     = findEventAtYPosition (newVisibleArea.getBottom ());
    const auto newEnd   = std::min (lastVisible + 2, static_cast<int> (eventList.getNumChildren ()));

    DBG ("firstVisible: " << newStart << " lastVisible: " << newEnd);
    juce::Range<int> newVisibleRange (newStart, newEnd);
    if (newVisibleRange == visibleEventRange)
        return;

    const auto currentStart = visibleEventRange.getStart ();
    const auto currentEnd   = visibleEventRange.getEnd ();

    // case 1: there's no overlap between the new visible range and the current visible range
    if (!newVisibleRange.intersects (visibleEventRange))
    {
        DBG ("***************** NO OVERLAP *****************");
        // clear all visible event views
        while (!visibleEventViews.empty ())
            hideEvent (InsertionPoint::bottom);
        // ensure clean state -- hideEvent decrements the range end but may leave
        // a non-empty start behind (e.g. [5,5)), reset to truly empty so that
        // the first displayEvent call below correctly reinitialises it.
        visibleEventRange = {};
        // add the new visible event views
        for (auto i { newStart }; i < newEnd; ++i)
        {
            auto eventView { createEventView (eventList[i], i, getWidth ()) };
            if (eventView == nullptr)
                continue;
            displayEvent (std::move (eventView), i, InsertionPoint::bottom);
        }
    }
    else
    {
        // case 2: changes at the beginning of the current visible range
        if (newStart > currentStart)
        {
            DBG ("*****DELETE EVENTS " << currentStart << ".." << newStart << " ABOVE THE CURRENT VISIBLE RANGE*****");
            const auto toRemove = std::min (newStart - currentStart,
                                            static_cast<int> (visibleEventViews.size ()));
            for (auto i = 0; i < toRemove; ++i)
                hideEvent (InsertionPoint::top);
        }
        else if (newStart < currentStart)
        {
            DBG ("*****ADD EVENTS " << newStart << ".." << currentStart << " ABOVE THE CURRENT VISIBLE RANGE*****");
            // add events above the current visible range (reverse order so front = newStart)
            for (auto i = currentStart - 1; i >= newStart; --i)
            {
                if (auto eventView { createEventView (eventList[i], i, getWidth ()) })
                    displayEvent (std::move (eventView), i, InsertionPoint::top);
            }
        }
        // case 3: changes at the end of the current visible range
        if (newEnd < currentEnd)
        {
            DBG ("*****DELETE EVENTS " << newEnd << ".." << currentEnd << " BELOW THE CURRENT VISIBLE RANGE*****");
            const auto toRemove = std::min (currentEnd - newEnd,
                                            static_cast<int> (visibleEventViews.size ()));
            for (auto i = 0; i < toRemove; ++i)
                hideEvent (InsertionPoint::bottom);
        }
        else if (newEnd > currentEnd)
        {
            DBG ("*****ADD EVENTS " << currentEnd << ".." << newEnd << " BELOW THE CURRENT VISIBLE RANGE*****");
            // add 1..n events below the current visible range
            for (auto i { currentEnd }; i < newEnd; ++i)
            {
                if (auto eventView { createEventView (eventList[i], i, getWidth ()) })
                    displayEvent (std::move (eventView), i, InsertionPoint::bottom);
            }
        }
    }
    // visibleEventRange is maintained incrementally by displayEvent/hideEvent;
    // do NOT override it here with newVisibleRange, which could claim a range
    // larger than what was actually populated (e.g. if any createEventView calls
    // returned nullptr, the deque would be shorter than the range implies).
}

int EventListView::findEventAtYPosition (int yPos) const
{
    if (eventPositions.empty ())
        return 0;

    // eventPositions[i] is the top-y of event i, in ascending order.
    // Find the last event whose top is <= yPos (i.e. the event that owns yPos).
    auto it = std::upper_bound (eventPositions.begin (), eventPositions.end (), yPos);
    if (it == eventPositions.begin ())
        return 0;
    return static_cast<int> (std::distance (eventPositions.begin (), std::prev (it)));
}
