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

#include "model/appContext.h"
#include "resizeHandle.h"
#include "model/persistentContext.h"
#include "model/runtimeContext.h"

class DataViewHeader : public juce::Component
{
public:
    DataViewHeader (AppContext& context);

    void paint (juce::Graphics& g) override;
    void resized () override;

    static constexpr int kHeaderHeight { 32 };

private:
    static constexpr int kDividerWidth { 5 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DataViewHeader)
    AppContext appContext;
    PersistentContext persistentContext;
    RuntimeContext runtimeContext;
    juce::Label timeLabel;
    juce::Label endpointLabel;
    juce::Label eventLabel;
    juce::Label dataLabel;
    ResizeHandle col1Resizer;
    ResizeHandle col2Resizer;
    ResizeHandle col3Resizer;
};
