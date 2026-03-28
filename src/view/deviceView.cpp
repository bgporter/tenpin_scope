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

#include "deviceView.h"

#include "model/midiEndpointProperties.h"
#include "palette.h"

///===========================================================================

DeviceView::DeviceView (AppContext& theAppContext)
: appContext { theAppContext }
, runtimeContext { appContext }
, midiProperties { runtimeContext }
, resizer { theAppContext, juce::Identifier { "sidebarWidth" } }
{
    addAndMakeVisible (resizer);
    rebuild ();

    // set up the callbacks....
    midiProperties.endpoints.onChildAdded   = [this] (juce::ValueTree&, int, int) { rebuild (); };
    midiProperties.endpoints.onChildRemoved = [this] (juce::ValueTree&, int, int) { rebuild (); };
    midiProperties.endpoints.onChildMoved   = [this] (juce::ValueTree&, int, int) { rebuild (); };
}

void DeviceView::paint (juce::Graphics& g)
{
    Palette palette { PersistentContext { appContext } };
    g.fillAll (palette.windowBackground.get ());
}

void DeviceView::resized ()
{
    auto bounds = getLocalBounds ();
    resizer.setBounds (bounds.removeFromRight (resizeHandleWidth));

    constexpr int endpointRowHeight = 50;
    for (auto& endpointView : endpointViews)
    {
        endpointView->setBounds (bounds.removeFromTop (endpointRowHeight));
    }
}

void DeviceView::rebuild ()
{
    endpointViews.clear ();
#if 0
    for (auto i { 0 }; i < midiProperties.getNumChildren (); ++i)
    {
        endpointViews.push_back (std::make_unique<EndpointView> (appContext, midiProperties[i]));
        addAndMakeVisible (endpointViews.back ().get ());
    }
#else
    for (const auto& endpoint : midiProperties.endpoints)
    {
        endpointViews.push_back (std::make_unique<EndpointView> (appContext, endpoint));
        addAndMakeVisible (endpointViews.back ().get ());
    }
#endif
    resized ();
}
