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

EventListView::EventListView (AppContext& theAppContext)
: appContext { theAppContext }
, runtimeContext { appContext }
, midiProperties { runtimeContext }
, eventList { midiProperties.midiEvents }
, handler { std::make_unique<EventListViewHandler> (appContext) }
, eventViewPool { appContext }
{
    eventList.onChildAdded      = [this] (juce::ValueTree& vt, int, int) { addEvent (vt); };
    eventList.onChildrenCleared = [this] () { clear (); };

    // Sync initial state if list already has events?
    // I'm not sure if this can happen, figure it out later.
    // iterate through the event list,
}

void EventListView::clear ()
{
    TRACE_ ("EventListView::clear");
    for (; !visibleEventViews.empty (); visibleEventViews.pop_front ())
    {
        eventViewPool.returnEventView (std::move (visibleEventViews.front ()));
    }
    visibleEventViews.clear ();
    eventPositions.clear ();
    eventPositions.push_back (0);
    setSize (getWidth (), 0);
}

void EventListView::addEvent (juce::ValueTree& vt)
{
    UmpEvent event (vt);
    auto eventView { eventViewPool.getEventView () };
    const auto result { handler->handle (event, eventList.getNumChildren (), eventView.get (), getWidth ()) };
    if (result != UmpHandler::Result::ok)
    {
        const auto fmt { juce::XmlElement::TextFormat ().singleLine ().withoutHeader () };
        WARN_ ({
            {   "msg",  "Failed to handle event"},
            { "event",      vt.toXmlString (fmt)},
            {"result", static_cast<int> (result)}
        });
        return;
    }

    // we always need to know where this new view should be positioned, and we
    // always need to grow the component to hold it, whether we are about to actually
    // display it or not.
    // calculate the y-position of the view & add it to the event positions

    const auto eventPosition = [this, &eventView] () -> int
    {
        if (eventPositions.empty ())
            return 0;
        return eventPositions.back () + eventView->getHeight ();
    }();
    eventPositions.push_back (eventPosition);
    setSize (getWidth (), eventPosition + eventView->getHeight ());

    if (shouldDisplayNewEvent ())
    {
        addAndMakeVisible (eventView.get ());
        eventView->setTopLeftPosition (0, eventPosition);
        DBG ("Adding event " << eventPositions.size () - 1 << " view at position: " << eventPosition);
        DBG ("event bounds: " << eventView->getBounds ().toString ());
        visibleEventViews.push_back (std::move (eventView));
        viewport->setViewPositionProportionately (0.0, 1.0);
    }
    else
    {
        // we only needed to create the view to calculate its size, so
        // we can return it to the pool.
        eventViewPool.returnEventView (std::move (eventView));
    }
}

void EventListView::widthChanged (int newWidth)
{
    DBG ("EventListView::widthChanged: " << newWidth);
    // for now, just update the width. We'll need to recalc the positions
    // which will also update the height.
    setSize (newWidth, getHeight ());
}

bool EventListView::shouldDisplayNewEvent () const
{
    return true;
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
    if (visibleArea == newVisibleArea)
        return;

    visibleArea = newVisibleArea;

    int firstVisible = findEventAtYPosition (newVisibleArea.getY ());
    int lastVisible  = findEventAtYPosition (newVisibleArea.getBottom ());
}

int EventListView::findEventAtYPosition (int yPos) const
{
    if (eventPositions.empty ())
        return 0;

    int prevPosition = 0;
    for (int i = 1; i < static_cast<int> (eventPositions.size ()); ++i)
    {
        const int currentPosition = eventPositions[i];
        if (yPos < currentPosition && yPos >= prevPosition)
            return i - 1;
        prevPosition = currentPosition;
    }
    return static_cast<int> (eventPositions.size ()) - 1;
}
