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

/**
 * @file VariantConverters.h
 * @brief A place to put commonly-used variant converters to prevent
 * duplications.
 *
 */

namespace juce
{
template <> struct VariantConverter<uint8_t>
{
    static uint8_t fromVar (const juce::var& v)
    {
        // force to int, then cast to our desired type.
        return static_cast<uint8_t> (static_cast<int> (v));
    }

    static juce::var toVar (uint8_t val) { return static_cast<int> (val); }
};

template <> struct VariantConverter<uint16_t>
{
    static uint16_t fromVar (const juce::var& v)
    {
        // force to int, then cast to our desired type.
        return static_cast<uint16_t> (static_cast<int> (v));
    }

    static juce::var toVar (uint16_t val) { return static_cast<int> (val); }
};

template <> struct VariantConverter<uint32_t>
{
    static uint32_t fromVar (const juce::var& v)
    {
        // force to int, then cast to the desired type.
        return static_cast<uint32_t> (static_cast<int> (v));
    }
    static juce::var toVar (uint32_t val) { return static_cast<int> (val); }
};

template <> struct VariantConverter<juce::MemoryBlock>
{
    static juce::MemoryBlock fromVar (const juce::var& v) { return *v.getBinaryData (); }

    static juce::var toVar (const juce::MemoryBlock& val) { return juce::var (val); }
};

template <> struct VariantConverter<juce::URL>
{
    static juce::URL fromVar (const juce::var& v)
    {
        // force to string, then cast to our desired type.
        return juce::URL { static_cast<juce::String> (v) };
    }

    static juce::var toVar (const juce::URL& val) { return val.toString (false); }
};

template <> struct VariantConverter<juce::File>
{
    static juce::File fromVar (const juce::var& v) { return juce::File { v.toString () }; }
    static juce::var toVar (const juce::File& f) { return f.getFullPathName (); }
};

/**
 * @brief store a string array as a comma-separated string. In the future we
 * can/should consider rewriting this as e.g. storing an array of `var`s, each
 * of which is a string (which would simplify/generalize cases where we need to
 * store arrays of strings that themselves contain commas)
 *
 */
template <> struct VariantConverter<juce::StringArray>
{
    static juce::StringArray fromVar (const juce::var& v)
    {
        return juce::StringArray::fromTokens (static_cast<juce::String> (v), ",", "");
    }

    static juce::var toVar (const juce::StringArray& sa) { return sa.joinIntoString (","); }
};

} // namespace juce