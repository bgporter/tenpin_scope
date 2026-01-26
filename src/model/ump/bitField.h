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

namespace UmpWords
{
const juce::Identifier data0Id { "data0" };
const juce::Identifier data1Id { "data1" };
const juce::Identifier data2Id { "data2" };
const juce::Identifier data3Id { "data3" };
} // namespace UmpWords

/**
 * @class BitField
 * @brief A class that represents a bit field within a larger value.
 */
template <typename T, int wordIndex, int dataWidth, int shiftBits> class BitField : public cello::ComputedValue<T>
{
public:
    using cello::ComputedValue<T>::operator=;
    constexpr static uint32_t mask = (dataWidth == 32) ? 0xFFFFFFFF : (1u << dataWidth) - 1;

    BitField (cello::Object& object, const juce::Identifier& id)
    : cello::ComputedValue<T> { object, id }
    {
        static_assert (wordIndex >= 0 && wordIndex < 4, "Word index must be between 0 and 3");
        static_assert (dataWidth > 0 && dataWidth <= 32, "Data width must be between 1 and 32");
        static_assert (shiftBits >= 0 && shiftBits < 32, "Shift bits must be between 0 and 31");
        static_assert (shiftBits + dataWidth <= 32, "Shift bits + data width must be less than or equal to 32");
        this->getImpl = [this] () -> T
        {
            const auto raw = getRawWord ();
            return static_cast<T> (raw >> shiftBits) & mask;
        };

        this->setImpl = [this] (const T& val)
        {
            const auto raw    = getRawWord ();
            const auto newRaw = (raw & ~(mask << shiftBits)) | (static_cast<uint32_t> (val) << shiftBits);
            setRawWord (newRaw);
        };
    }

    /**
     * @brief Get the identifier of the data word for this bit field.
     *
     * @return juce::Identifier
     */
    static juce::Identifier getDataId ()
    {
        // clang-format off
        switch (wordIndex)
        {
            case 0: return UmpWords::data0Id;
            case 1: return UmpWords::data1Id;
            case 2: return UmpWords::data2Id;
            case 3: return UmpWords::data3Id;
            default: jassertfalse; return {};
        }
        // clang-format on
    }

    /**
     * @brief Get the raw word value for this bit field.
     *
     * @return uint32_t
     */
    uint32_t getRawWord () const { return this->object.template getattr<uint32_t> (getDataId (), 0); }

    /**
     * @brief Set the raw word value for this bit field.
     *
     * @param raw
     */
    void setRawWord (uint32_t raw) { this->object.template setattr<uint32_t> (getDataId (), raw); }
};

// clang-format off
/**
 * @brief A macro to create a BitField member of a cello::Object.
 */
#define MAKE_BITFIELD(type, name, wordIndex, dataWidth, shiftBits) \
    static const inline juce::Identifier name##Id { #name };         \
    BitField<type, wordIndex, dataWidth, shiftBits> name { *this, name##Id }
// clang-format on
