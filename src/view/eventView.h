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

#include "model/appContext.h"

class EventView : public juce::Component
{
public:
    EventView (AppContext& theAppContext, juce::String theEventDescription);

    void paint (juce::Graphics& g) override;

    void resized () override;

    int getContentHeight () const
    {
        // Variable height based on width for testing virtual scrolling
        int w = getWidth ();
        if (w < 200)
            return 32;
        else if (w < 600)
            return 24;
        else
            return 16;
    }

    juce::String getDescription () const { return eventDescription; }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EventView)
    AppContext appContext;
    juce::String eventDescription;
    juce::Label eventDescriptionLabel;
};
