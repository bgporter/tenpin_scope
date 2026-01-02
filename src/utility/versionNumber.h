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

#include "JuceHeader.h"
#include <compare>

struct VersionNumber
{
    int major { 0 };
    int minor { 0 };
    int patch { 0 };

    /**
     * @brief Parse a string in the format we're using (x.y.z) and return
     * the resulting VersionNumber object.
     *
     * @param numberStr
     * @return VersionNumber
     */
    static VersionNumber getVersionNumberFromString (const juce::String& numberStr);
    juce::String toString () const;
    operator juce::String () const { return toString (); }

    auto operator<=> (const VersionNumber&) const = default;
};

namespace juce
{
/** @warning Not actually made by JUCE! */
template <> struct VariantConverter<VersionNumber> final
{
    static VersionNumber fromVar (const juce::var& v)
    {
        return VersionNumber::getVersionNumberFromString (v.toString ());
    }

    static juce::var toVar (const VersionNumber& ver) { return ver.toString (); }
};
} // namespace juce
