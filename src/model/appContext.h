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

#include "persistentContext.h"
#include "runtimeContext.h"

class AppContext : public cello::Object
{
public:
    static const inline juce::Identifier type { "root" };

    /**
     * @brief Construct a new AppContext object. We create a default RuntimeContext
     * as a child of this object. After loading a PersistentContext object from disk,
     * we append it to this object as a child.

     * @param file The file to load the PersistentContext from. If the file does not
     * exist, we create it in the user application data directory.
     */
    AppContext (juce::File file)
    : cello::Object (type.toString (), nullptr)
    , runtimeContext (*this)
    {
        auto persistent { PersistentContext { file } };
        append (&persistent);
    }

    ~AppContext () = default;

private:
    RuntimeContext runtimeContext;
};
