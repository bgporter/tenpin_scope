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
    Yamaha, // middle C = C3
    Roland  // middle C = C4
};

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
        const int intValue = static_cast<int> (v);
        if (intValue < 0 || intValue > 1)
            return OctaveType::Yamaha; // default value
        return static_cast<OctaveType> (intValue);
    }
    static var toVar (const OctaveType& octaveType)
    {
        // return the value as a string for better readability in the prefs file.
        const juce::String name = getOctaveTypeName (octaveType);
        return var (name);
    }