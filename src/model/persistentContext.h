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

class PersistentContext : public cello::Object
{
public:
    static const inline juce::Identifier type { ProjectInfo::projectName };
    /**
     * @brief Construct a new PersistentContext object
     *
     * @param parentOrSelf If the type of the object we're passed matches our type,
     * we'll wrap that directly; otherwise we will look for (or create) a defaulted
     * instance of our type as a child of the passed object.
     */
    PersistentContext (cello::Object& parentOrSelf)
    : cello::Object { type.toString (), parentOrSelf }
    {
        sidebarWidth.onSet = [] (int newValue) { return std::clamp (newValue, 150, 400); };
    }

    PersistentContext (const juce::File& file)
    : cello::Object { type.toString (), file }
    {
    }

    juce::Result save (const juce::File& file)
    {
        auto ok = cello::Object::save (file);
        if (!ok)
        {
            juce::String msg { "Failed to save file " + file.getFullPathName () };
            msg << ": " << ok.getErrorMessage ();
            jassertfalse;
            return juce::Result::fail (msg);
        }
        clearUndoHistory ();
        return juce::Result::ok ();
    }

    juce::Result saveIfNeeded (const juce::File& file)
    {
        if (canUndo ())
        {
            return save (file);
        }
        return juce::Result::ok ();
    }

    MAKE_VALUE_MEMBER (juce::String, windowState, {});
    MAKE_VALUE_MEMBER (int, sidebarWidth, 200);
};
