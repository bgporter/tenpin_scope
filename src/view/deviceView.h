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

#include "endpointView.h"
#include "model/appContext.h"
#include "model/midiProperties.h"
#include "model/persistentContext.h"
#include "model/runtimeContext.h"
#include "resizeHandle.h"

class DeviceView : public juce::Component
{
public:
    DeviceView (AppContext& theAppContext);
    void paint (juce::Graphics& g) override;
    void resized () override;

    /**
     * @brief Create a child component for each endpoint that we've
     * known about, whether it's active now or not.
     */
    void rebuild ();

private:
    static const inline int resizeHandleWidth { 5 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeviceView)
    AppContext appContext;
    RuntimeContext runtimeContext;
    PersistentContext persistentContext;
    MidiProperties midiProperties;
    ResizeHandle resizer;
    std::vector<std::unique_ptr<EndpointView>> endpointViews;
};