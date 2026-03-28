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

#include "dataViewHeader.h"

#include "palette.h"

DataViewHeader::DataViewHeader (AppContext& context)
: appContext { context }
, persistentContext { appContext }
, runtimeContext { appContext }
, col1Resizer { runtimeContext.col1Width, persistentContext.col1Width, persistentContext.dragging }
, col2Resizer { runtimeContext.col2Width, persistentContext.col2Width, persistentContext.dragging }
{
    for (auto* label : { &timeLabel, &endpointLabel, &dataLabel })
    {
        label->setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (label);
    }
    timeLabel.setText ("Time", juce::dontSendNotification);
    endpointLabel.setText ("Endpoint", juce::dontSendNotification);
    dataLabel.setText ("Data", juce::dontSendNotification);

    addAndMakeVisible (col1Resizer);
    addAndMakeVisible (col2Resizer);

    // Seed runtime context with persisted values so initial layout is correct.
    runtimeContext.setattr<int> ("col1Width", persistentContext.col1Width);
    runtimeContext.setattr<int> ("col2Width", persistentContext.col2Width);

    runtimeContext.col1Width.onPropertyChange ([this] (const juce::Identifier&) { resized (); });
    runtimeContext.col2Width.onPropertyChange ([this] (const juce::Identifier&) { resized (); });
}

void DataViewHeader::paint (juce::Graphics& g)
{
    Palette palette { PersistentContext { appContext } };
    g.fillAll (palette.windowBackground.get ());
}

void DataViewHeader::resized ()
{
    const int col1Width    = runtimeContext.col1Width;
    const int col2Width    = runtimeContext.col2Width;
    const int half         = kDividerWidth / 2;
    const int handle1Start = col1Width - half;
    const int handle2Start = col1Width + kDividerWidth + col2Width - half;

    timeLabel.setBounds (0, 0, handle1Start, getHeight ());
    col1Resizer.setBounds (handle1Start, 0, kDividerWidth, getHeight ());
    endpointLabel.setBounds (handle1Start + kDividerWidth, 0, col2Width, getHeight ());
    col2Resizer.setBounds (handle2Start, 0, kDividerWidth, getHeight ());
    dataLabel.setBounds (handle2Start + kDividerWidth, 0, getWidth () - handle2Start - kDividerWidth, getHeight ());
}
