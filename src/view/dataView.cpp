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

#include "dataView.h"

#include "palette.h"

DataView::DataView (AppContext& theAppContext)
: appContext { theAppContext }
, runtimeContext { appContext }
, midiProperties { runtimeContext }
, dataViewHandler { *this }
{
    for (const auto& endpoint : midiProperties)
    {
        addEndpoint (endpoint, 0);
    }
    midiProperties.onChildAdded = [this] (juce::ValueTree& vt, int _, int index) { addEndpoint (vt, index); };

    midiProperties.onChildRemoved = [this] (juce::ValueTree& vt, int index, int _) { removeEndpoint (vt, index); };
}

void DataView::paint (juce::Graphics& g)
{
    Palette palette { PersistentContext { appContext } };
    g.fillAll (palette.windowBackground.get ());
}

void DataView::resized () {}

void DataView::addEndpoint (juce::ValueTree vt, int index)
{
    auto endpointProperties = std::make_unique<MidiEndpointProperties> (vt);

    endpointProperties->received.onChildAdded = [this] (juce::ValueTree& vt, int, int)
    { dataViewHandler.handle (UmpEvent (vt)); };
    endpoints.push_back (std::move (endpointProperties));
    DBG ("========= endpoint added: " << endpoints.back ()->name.get ());
}

void DataView::removeEndpoint (juce::ValueTree vt, int index)
{
    auto endpointIdString = MidiEndpointProperties { vt }.endpointIdString.get ();
    endpoints.erase (std::remove_if (endpoints.begin (), endpoints.end (),
                                     [&] (const auto& endpoint)
                                     { return endpoint->endpointIdString.get () == endpointIdString; }),
                     endpoints.end ());
    DBG ("========= endpoint removed: " << endpointIdString);
}
