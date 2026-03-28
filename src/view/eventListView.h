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

#include "eventListViewHandler.h"
#include "eventView.h"
#include "model/appContext.h"
#include "model/eventList.h"
#include "model/midiProperties.h"
#include "model/persistentContext.h"
#include "model/runtimeContext.h"

enum class EventListChange
{
    appended,
    cleared
};

class EventListView : public juce::Component
{
public:
    EventListView (AppContext& theAppContext);

    void paint (juce::Graphics& g) override;

    void resized () override;

    /**
     * @brief Removes all EventViews from the list/display, returning them to the pool
     * and clears the event positions.
     *
     */
    void clear ();

    /**
     * @brief Called when viewport scrolls to update visible views.
     * We need to:
     * - map from the visible area to the event indices
     * - remove any views that are no longer needed
     * - create any views that are needed, and add them to the visible event views
     * - position the views
     */
    void visibleAreaChanged (const juce::Rectangle<int>& newVisibleArea);

    /**
     * @brief Called when the width of the list changes, so we can recalculate the positions of the events.
     */
    void widthChanged (int newWidth);

    /**
     * @brief Force a full rebuild at the current width. Called when column widths change
     * so that event heights (which depend on wrap points) are recalculated.
     */
    void forceRebuild ();

    /**
     * @brief Reflow only the currently-visible EventViews. Called during column
     * drag for a fast O(visible) preview. eventPositions is not updated; the
     * full forceRebuild() corrects everything on mouseUp.
     */
    void refreshVisibleViews ();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EventListView)

    int findEventAtYPosition (int yPos) const;
    void createViewsInRange (const juce::Range<int>& range);

    /**
     * @brief Called when the parent hierarchy changes, so we can find the
     * viewport that owns us.
     */
    void parentHierarchyChanged () override
    {
        if (viewport == nullptr)
            viewport = findParentComponentOfClass<juce::Viewport> ();
    }

    /**
     * @brief Determine if the new event should be displayed.
     */
    bool shouldDisplayNewEvent () const;
    void addEvent (juce::ValueTree vt);
    enum class InsertionPoint
    {
        top,
        bottom
    };

    void displayEvent (std::unique_ptr<EventView> eventView, int index, InsertionPoint insertionPoint);
    void hideEvent (InsertionPoint insertionPoint);

    std::unique_ptr<EventView> createEventView (juce::ValueTree vt, int index, int width);
    class EventViewPool
    {
    public:
        EventViewPool (AppContext& theAppContext)
        : appContext (theAppContext)
        {
        }
        ~EventViewPool () = default;

        std::unique_ptr<EventView> getEventView ()
        {
            if (pool.empty ())
            {
                DBG ("EventViewPool: allocating (pool empty)");
                return std::make_unique<EventView> (appContext);
            }
            DBG ("EventViewPool: reusing from pool (size=" << pool.size () << ")");
            auto eventView = std::move (pool.top ());
            pool.pop ();
            return eventView;
        }

        void returnEventView (std::unique_ptr<EventView> view)
        {
            view->reset ();
            DBG ("EventViewPool: returning to pool (size now=" << (pool.size () + 1) << ")");
            pool.push (std::move (view));
        }

    private:
        std::stack<std::unique_ptr<EventView>> pool;
        AppContext appContext;
    };

    AppContext appContext;
    RuntimeContext runtimeContext;
    PersistentContext persistentContext;
    MidiProperties midiProperties;
    EventList eventList;

    juce::Viewport* viewport { nullptr };
    std::unique_ptr<class EventListViewHandler> handler;
    EventViewPool eventViewPool;
    std::deque<std::unique_ptr<EventView>> visibleEventViews;
    juce::Range<int> visibleEventRange;
    juce::Rectangle<int> visibleArea;

    // upper-left y-position metadata (persistent for ALL events),
    // recalculated as needed.
    std::vector<int> eventPositions;

    // Guard against visibleAreaChanged firing mid-rebuild in widthChanged
    bool isRecalculating { false };
};
