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

#include "model/ump/umpEvent.h"

/**
 * @brief Returns a concise UI string for a SysEx packet status.
 */
juce::String getSysexStatusName (SysexStatus status);

/**
 * @brief Returns a concise UI string for a MIDI Time Code quarter-frame message type.
 */
juce::String getMtcMessageTypeName (MtcMessageType mtcType);

/**
 * @brief Gets the name of a controller based on its number.
 *
 * @param controllerNumber (must be in the range 0-127)
 * @return juce::String
 */
juce::String getControllerName (int controllerNumber);

/**
 * In the early days of MIDI, there was no standard for which octave number
 * corresponded to middle C, so different manufacturers adopted different
 * conventions. This function allows you to specify which convention to
 * use when getting note names.
 *
 * (There's an even rarer third convention where middle C is C5, but
 * we're not supporting that.)
 *
 */
enum class OctaveType
{
    Yamaha = 1, // middle C = C3
    Roland      // middle C = C4
};

enum class ValueFormatType
{
    Integer = 1, // format as a decimal integer (e.g. "64")
    Hex,         // format as a hexadecimal integer (e.g. "0x40")
    Float,       // format as a float between 0..1 or -1..1 (e.g. "0.5" or "-0.5")
    Midi,        // format as a (high-res) MIDI value 0..127 with fractional part if present.
    Percent      // format as a percentage 0..100 (e.g. "50.00%")
};

inline juce::String getValueFormatTypeName (ValueFormatType formatType)
{
    switch (formatType)
    {
        case ValueFormatType::Integer:
            return "Integer";
        case ValueFormatType::Hex:
            return "Hex";
        case ValueFormatType::Float:
            return "Float";
        case ValueFormatType::Midi:
            return "Midi";
        case ValueFormatType::Percent:
            return "Percent";
        default:
            jassertfalse;
            return "Unknown";
    }
}

inline juce::String getNoteAttributeName (int attributeNumber)
{
    const juce::String name = [attributeNumber] ()
    {
        switch (attributeNumber)
        {
            case 0:
                return "none";
            case 1:
                return "Mfr Specific";
            case 2:
                return "Profile Specific";
            case 3:
                return "Pitch 7.9";
            default:
                jassertfalse;
                return "Reserved";
        }
    }();
    return juce::String (attributeNumber) + " (" + name + ")";
}

juce::String formatValue (uint32_t value, int bitWidth, ValueFormatType formatType, int precision = 2,
                          float formattedMin = 0.f, float formattedMax = 1.f);

/**
 * @brief Formats a time value in seconds as a compact string, expanding
 * the number of displayed fields only as needed:
 *   s.mmm / ss.mmm / M:ss.mmm / MM:ss.mmm / H:ss.mmm / HH:ss.mmm
 *
 * @param seconds Non-negative elapsed time in seconds.
 * @return juce::String
 */
juce::String formatTime (double seconds);

/**
 * @brief Gets the name of a note based on its MIDI note number, using
 * the specified octave convention.
 *
 * @param noteNumber
 * @param octaveType
 * @return juce::String
 */
juce::String getNoteName (int noteNumber, OctaveType octaveType = OctaveType::Yamaha);

juce::String getOctaveTypeName (OctaveType octaveType);

// a JUCE::VariantConverter for OctaveType, so we can store it in the PersistentContext.
namespace juce
{
template <> struct VariantConverter<OctaveType>
{
    static OctaveType fromVar (const var& v)
    {
        const juce::String name = v.toString ();
        if (name == getOctaveTypeName (OctaveType::Roland))
            return OctaveType::Roland;
        return OctaveType::Yamaha; // default value
    }
    static var toVar (const OctaveType& octaveType)
    {
        // return the value as a string for better readability in the prefs file.
        const juce::String name = getOctaveTypeName (octaveType);
        return var (name);
    }
};

template <> struct VariantConverter<ValueFormatType>
{
    static ValueFormatType fromVar (const var& v)
    {
        const juce::String name = v.toString ();
        if (name == getValueFormatTypeName (ValueFormatType::Hex))
            return ValueFormatType::Hex;
        if (name == getValueFormatTypeName (ValueFormatType::Float))
            return ValueFormatType::Float;
        if (name == getValueFormatTypeName (ValueFormatType::Midi))
            return ValueFormatType::Midi;
        if (name == getValueFormatTypeName (ValueFormatType::Percent))
            return ValueFormatType::Percent;
        return ValueFormatType::Integer; // default value
    }
    static var toVar (const ValueFormatType& formatType)
    {
        // return the value as a string for better readability in the prefs file.
        const juce::String name = getValueFormatTypeName (formatType);
        return var (name);
    }
};
} // namespace juce