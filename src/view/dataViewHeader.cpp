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
{
    for (auto* label : { &timeLabel, &endpointLabel, &eventLabel, &dataLabel })
    {
        label->setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (label);
    }
    timeLabel.setText ("Time", juce::dontSendNotification);
    endpointLabel.setText ("Endpoint", juce::dontSendNotification);
    eventLabel.setText ("Event", juce::dontSendNotification);
    dataLabel.setText ("Data", juce::dontSendNotification);

    // Seed runtime context with persisted values so initial layout is correct.
    runtimeContext.col1Width = persistentContext.eventViewContext.col1Width.get ();
    runtimeContext.col2Width = persistentContext.eventViewContext.col2Width.get ();
    runtimeContext.col3Width = persistentContext.eventViewContext.col3Width.get ();

    runtimeContext.col1Width.onPropertyChange ([this] (const juce::Identifier&) { resized (); });
    runtimeContext.col2Width.onPropertyChange ([this] (const juce::Identifier&) { resized (); });
    runtimeContext.col3Width.onPropertyChange ([this] (const juce::Identifier&) { resized (); });
    runtimeContext.sidebarWidth.onPropertyChange ([this] (const juce::Identifier&) { resized (); });
}

void DataViewHeader::paint (juce::Graphics& g)
{
    Palette palette { PersistentContext { appContext } };
    g.fillAll (palette.windowBackground.get ());

    g.setColour (palette.defaultText.get ());
    g.drawHorizontalLine (getHeight () - 1, 0.0f, static_cast<float> (getWidth ()));
}

void DataViewHeader::resized ()
{
    const int xBase     = runtimeContext.sidebarWidth;
    const int col1Width = runtimeContext.col1Width;
    const int col2Width = runtimeContext.col2Width;
    const int col3Width = runtimeContext.col3Width;
    const int half      = kDividerWidth / 2;
    const int h1        = xBase + col1Width - half;
    const int h2        = h1 + kDividerWidth + col2Width;
    const int h3        = h2 + kDividerWidth + col3Width;

    timeLabel.setBounds (xBase, 0, h1 - xBase, getHeight ());
    endpointLabel.setBounds (h1 + kDividerWidth, 0, col2Width, getHeight ());
    eventLabel.setBounds (h2 + kDividerWidth, 0, col3Width, getHeight ());
    dataLabel.setBounds (h3 + kDividerWidth, 0, getWidth () - h3 - kDividerWidth, getHeight ());
}
