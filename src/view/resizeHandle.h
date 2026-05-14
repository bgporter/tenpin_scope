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

class ResizeHandle : public juce::Component,
                     public juce::Timer
{
public:
    enum class LinePosition
    {
        left,
        centre,
        right
    };

    ResizeHandle (cello::Value<int>& rcValue,
                  cello::Value<int>& pcValue,
                  cello::Value<bool>& dragging,
                  LinePosition linePosition = LinePosition::centre);
    ~ResizeHandle () override = default;

    void paint (juce::Graphics& g) override;

    void mouseDown (const juce::MouseEvent& e) override;
    void mouseUp (const juce::MouseEvent& e) override;
    void mouseDrag (const juce::MouseEvent& e) override;
    void mouseEnter (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent& e) override;

    void timerCallback () override;

    void setHeaderHeight (int h) { headerHeight = h; }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResizeHandle)
    cello::Value<int>& rcValue;
    cello::Value<int>& pcValue;
    cello::Value<bool>& dragging;
    LinePosition linePosition;
    int headerHeight { 0 };
    int dragStartValue { 0 };
    bool isHovered { false };
    bool isBeingDragged { false };
};
