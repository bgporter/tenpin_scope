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

#include "resizeHandle.h"

ResizeHandle::ResizeHandle (cello::Value<int>& rcValue_,
                             cello::Value<int>& pcValue_,
                             cello::Value<bool>& dragging_)
: rcValue { rcValue_ }
, pcValue { pcValue_ }
, dragging { dragging_ }
{
}

void ResizeHandle::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void ResizeHandle::mouseDown (const juce::MouseEvent&)
{
    dragStartValue = pcValue.get ();
    dragging       = true;
}

void ResizeHandle::mouseUp (const juce::MouseEvent&)
{
    pcValue  = rcValue.get ();
    dragging = false;
}

void ResizeHandle::mouseDrag (const juce::MouseEvent& e)
{
    rcValue = dragStartValue + e.getDistanceFromDragStartX ();
}
