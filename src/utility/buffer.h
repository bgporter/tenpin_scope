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
#include <vector>

/**
 * @brief Append-capable byte buffer that can be stored in a juce::ValueTree
 * via juce::var. Derives from ReferenceCountedObject so that Buffer::Ptr
 * (a ReferenceCountedObjectPtr) round-trips cleanly through juce::var.
 */
class Buffer : public juce::ReferenceCountedObject
{
public:
    using Ptr = juce::ReferenceCountedObjectPtr<Buffer>;

    Buffer () = default;

    void append (uint8_t byte) { data_.push_back (byte); }

    void append (const uint8_t* data, size_t count)
    {
        data_.insert (data_.end (), data, data + count);
    }

    size_t size () const { return data_.size (); }
    bool empty () const { return data_.empty (); }

    auto begin () { return data_.begin (); }
    auto end () { return data_.end (); }
    auto cbegin () const { return data_.cbegin (); }
    auto cend () const { return data_.cend (); }

    uint8_t& operator[] (size_t index) { return data_[index]; }
    const uint8_t& operator[] (size_t index) const { return data_[index]; }

private:
    std::vector<uint8_t> data_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Buffer)
};

namespace juce
{
template <> struct VariantConverter<Buffer::Ptr>
{
    static Buffer::Ptr fromVar (const juce::var& v)
    {
        return Buffer::Ptr (static_cast<Buffer*> (v.getObject ()));
    }

    static juce::var toVar (Buffer::Ptr ptr) { return juce::var (ptr.get ()); }
};
} // namespace juce
