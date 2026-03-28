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

#include "model/persistentContext.h"
#include "model/runtimeContext.h"

ResizeHandle::ResizeHandle (AppContext& context, juce::Identifier id)
: appContext { context }
, valueId { id }
{
}

void ResizeHandle::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black);
}

void ResizeHandle::mouseDown (const juce::MouseEvent&)
{
    PersistentContext pc { appContext };
    dragStartValue = pc.getattr<int> (valueId, 0);
    pc.dragging    = true;
}

void ResizeHandle::mouseUp (const juce::MouseEvent&)
{
    RuntimeContext rc { appContext };
    PersistentContext pc { appContext };
    pc.setattr<int> (valueId, rc.getattr<int> (valueId, dragStartValue));
    pc.dragging = false;
}

void ResizeHandle::mouseDrag (const juce::MouseEvent& e)
{
    RuntimeContext rc { appContext };
    const auto dragDelta { e.getDistanceFromDragStartX () };
    rc.setattr<int> (valueId, dragStartValue + dragDelta);
}
