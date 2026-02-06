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

#pragma once

#include <JuceHeader.h>
#include <map>

#include "eventView.h"
#include "model/appContext.h"

class EventListView : public juce::Component
{
public:
    EventListView (AppContext& theAppContext);

    void paint (juce::Graphics& g) override;

    void resized () override;

    void addEventView (std::unique_ptr<EventView> eventView);

    /**
     * @brief respond to resize events affecting the viewport that owns this component.
     */
    void parentSizeChanged () override;

    /**
     * @brief Calculate the sum of the heights of our contained components.
     *
     * @return int total required height.
     */
    int getContentHeight () const;

    /**
     * @brief Called when viewport scrolls to update visible views
     */
    void visibleAreaChanged (const juce::Rectangle<int>& newVisibleArea);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EventListView)

    // Helper methods
    int findEventAtYPosition (int yPos) const;
    void cullViewsOutsideRange (const juce::Range<int>& keepRange);
    void createViewsInRange (const juce::Range<int>& range);
    void positionView (int index);
    void recalculateCumulativeHeightsFrom (int startIndex);
    void updateContentSize ();
    bool isInBufferZone (int eventIndex) const;

    AppContext appContext;

    // Height metadata (persistent for ALL events)
    std::vector<int> eventHeights;       // One height per event
    std::vector<int> cumulativeHeights;  // Prefix sums for O(1) position lookup
    std::vector<juce::String> eventData; // Store event descriptions to recreate views

    // Active views (sparse, only visible + buffer)
    std::map<int, std::unique_ptr<EventView>> activeViews; // Index → View

    // Viewport tracking
    int bufferZoneEvents { 100 };         // Keep ±100 events outside viewport
    juce::Range<int> currentVisibleRange; // Currently visible event indices
};
