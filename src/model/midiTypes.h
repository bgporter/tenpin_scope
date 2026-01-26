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

#include "utility/logger.h"

/**
 * @brief A class that represents a MIDI nibble, clamped to the range 0-15.
 */
class MidiNibble
{
public:
    static const inline int maxValue = 15;
    MidiNibble (int theValue)
    {
        if (theValue < 0 || theValue > maxValue)
            ERROR_ ("value must be between 0 and 15");
        value = std::clamp (theValue, 0, maxValue);
    }

    int get () const { return value; }
    operator int () const { return get (); }

private:
    int value;
};

/**
 * @brief A class that represents a MIDI (7-bit) data byte, clamped to the range 0-127.
 */
class MidiByte
{
public:
    static const inline int maxValue = 127;
    MidiByte (int theValue)
    {
        if (theValue < 0 || theValue > maxValue)
            ERROR_ ("value must be between 0 and 127");
        value = std::clamp (theValue, 0, maxValue);
    }
    int get () const { return value; }
    operator int () const { return get (); }

private:
    int value;
};

/**
 * @brief A class that represents a MIDI (14-bit) data word, clamped to the range 0-16383.
 */
class MidiWord
{
public:
    static const inline int maxValue = 16383;
    MidiWord (int theValue)
    {
        if (theValue < 0 || theValue > maxValue)
            ERROR_ ("value must be between 0 and 16383");
        value = std::clamp (theValue, 0, maxValue);
    }

    MidiWord (MidiByte lsb, MidiByte msb) { value = (msb.get () << 7) | lsb.get (); }
    int get () const { return value; }
    operator int () const { return get (); }

    int getLsb () const { return value & 0x7F; }
    int getMsb () const { return (value >> 7) & 0x7F; }

private:
    int value;
};

/**
 * @brief A class that represents a MIDI (28-bit) data quad, clamped to the range 0-268435455.
 */
class MidiLong
{
public:
    static const inline int maxValue = 268435455;
    MidiLong (int theValue)
    {
        if (theValue < 0 || theValue > maxValue)
            ERROR_ ("value must be between 0 and 268435455");
        value = std::clamp (theValue, 0, maxValue);
    }

    MidiLong (MidiByte lsb, MidiByte byte2, MidiByte byte3, MidiByte msb)
    {
        value = (msb.get () << 21) | (byte3.get () << 14) | (byte2.get () << 7) | lsb.get ();
    }

    int get () const { return value; }
    operator int () const { return get (); }

    int getLsb () const { return value & 0x7F; }
    int getByte2 () const { return (value >> 7) & 0x7F; }
    int getByte3 () const { return (value >> 14) & 0x7F; }
    int getMsb () const { return (value >> 21) & 0x7F; }

private:
    int value;
};

class MidiUnipolarFloat
{
public:
    MidiUnipolarFloat (float theValue)
    {
        if (theValue < 0.0f || theValue > 1.0f)
            ERROR_ ("value must be between 0.0f and 1.0f");
        value = std::clamp (theValue, 0.0f, 1.0f);
    }
    float get () const { return value; }
    operator float () const { return get (); }

private:
    float value;
};

class MidiBipolarFloat
{
public:
    MidiBipolarFloat (float theValue)
    {
        if (theValue < -1.0f || theValue > 1.0f)
            ERROR_ ("value must be between -1.0f and 1.0f");
        value = std::clamp (theValue, -1.0f, 1.0f);
    }
    float get () const { return value; }
    operator float () const { return get (); }

private:
    float value;
};