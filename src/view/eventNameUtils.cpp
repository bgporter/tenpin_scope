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

#include "eventNameUtils.h"

#include <string>
#include <vector>
namespace
{

const std::vector<std::string> controllerNames = {
    "Bank Select",           // 0
    "Modulation Wheel",      // 1
    "Breath Controller",     // 2
    "",                      // 3 (Undefined)
    "Foot Controller",       // 4
    "Portamento Time",       // 5
    "Data Entry MSB",        // 6
    "Main Volume",           // 7
    "Balance",               // 8
    "",                      // 9 (Undefined)
    "Pan",                   // 10
    "Expression",            // 11
    "Effect Control 1",      // 12
    "Effect Control 2",      // 13
    "",                      // 14
    "",                      // 15 (Undefined)
    "General Purpose 1",     // 16
    "General Purpose 2",     // 17
    "General Purpose 3",     // 18
    "General Purpose 4",     // 19
    "",                      // 20
    "",                      // 21
    "",                      // 22
    "",                      // 23 (Undefined)
    "",                      // 24
    "",                      // 25
    "",                      // 26
    "",                      // 27 (Undefined)
    "",                      // 28
    "",                      // 29
    "",                      // 30
    "",                      // 31 (Undefined)
    "Bank Select LSB",       // 32
    "Modulation Wheel LSB",  // 33
    "Breath Controller LSB", // 34
    "35 (undefined 3 LSB)",  // 35
    "Foot Controller LSB",   // 36
    "Portamento Time LSB",   // 37
    "Data Entry LSB",        // 38
    "Main Volume LSB",       // 39
    "Balance LSB",           // 40
    "41 (undefined 9 LSB)",  // 41
    "Pan LSB",               // 42
    "Expression LSB",        // 43
    "Effect Control 1 LSB",  // 44
    "Effect Control 2 LSB",  // 45
    "46 (undefined 14 LSB)", // 46
    "47 (undefined 15 LSB)", // 47
    "General Purpose 1 LSB", // 48
    "General Purpose 2 LSB", // 49
    "General Purpose 3 LSB", // 50
    "General Purpose 4 LSB", // 51
    "52 (undefined 20 LSB)", // 52
    "53 (undefined 21 LSB)", // 53
    "54 (undefined 22 LSB)", // 54
    "55 (undefined 23 LSB)", // 55
    "56 (undefined 24 LSB)", // 56
    "57 (undefined 25 LSB)", // 57
    "58 (undefined 26 LSB)", // 58
    "59 (undefined 27 LSB)", // 59
    "60 (undefined 28 LSB)", // 60
    "61 (undefined 29 LSB)", // 61
    "62 (undefined 30 LSB)", // 62
    "63 (undefined 31 LSB)", // 63
    "Sustain Pedal",         // 64
    "Portamento",            // 65
    "Sostenuto",             // 66
    "Soft Pedal",            // 67
    "Legato Footswitch",     // 68
    "Hold 2",                // 69
    "Sound Variation",       // 70
    "Resonance",             // 71
    "Release Time",          // 72
    "Attack Time",           // 73
    "Brightness",            // 74
    "Sound Controller 6",    // 75
    "Sound Controller 7",    // 76
    "Sound Controller 8",    // 77
    "Sound Controller 9",    // 78
    "Sound Controller 10",   // 79
    "General Purpose 5",     // 80
    "General Purpose 6",     // 81
    "General Purpose 7",     // 82
    "General Purpose 8",     // 83
    "Portamento Control",    // 84
    "",                      // 85
    "",                      // 86
    "",                      // 87
    "",                      // 88
    "",                      // 89
    "",                      // 90 (Undefined)
    "Effects 1 Depth",       // 91 (Reverb)
    "Effects 2 Depth",       // 92 (Tremolo)
    "Effects 3 Depth",       // 93 (Chorus)
    "Effects 4 Depth",       // 94 (Detune)
    "Effects 5 Depth",       // 95 (Phaser)
    "Data Increment",        // 96
    "Data Decrement",        // 97
    "NRPN LSB",              // 98
    "NRPN MSB",              // 99
    "RPN LSB",               // 100
    "RPN MSB",               // 101
    "",                      // 102
    "",                      // 103
    "",                      // 104
    "",                      // 105
    "",                      // 106
    "",                      // 107
    "",                      // 108
    "",                      // 109
    "",                      // 110
    "",                      // 111
    "",                      // 112
    "",                      // 113
    "",                      // 114
    "",                      // 115
    "",                      // 116
    "",                      // 117
    "",                      // 118
    "",                      // 119 (Undefined)
    "All Sound Off",         // 120
    "Reset All Controllers", // 121
    "Local Control",         // 122
    "All Notes Off",         // 123
    "Omni Mode Off",         // 124
    "Omni Mode On",          // 125
    "Mono Mode On",          // 126
    "Poly Mode On"           // 127
};

static const std::array<juce::String, 12> noteNames = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

} // namespace

juce::String getSysexStatusName (SysexStatus status)
{
    switch (status)
    {
        case SysexStatus::complete:  return "Complete";
        case SysexStatus::start:     return "Start";
        case SysexStatus::continue_: return "Continue";
        case SysexStatus::end:       return "End";
        default:
            jassertfalse;
            return "Unknown";
    }
}

juce::String getMtcMessageTypeName (MtcMessageType mtcType)
{
    switch (mtcType)
    {
        case MtcMessageType::frameCountLsb:    return "Frame LSB";
        case MtcMessageType::frameCountMsb:    return "Frame MSB";
        case MtcMessageType::secondsCountLsb:  return "Seconds LSB";
        case MtcMessageType::secondsCountMsb:  return "Seconds MSB";
        case MtcMessageType::minutesCountLsb:  return "Minutes LSB";
        case MtcMessageType::minutesCountMsb:  return "Minutes MSB";
        case MtcMessageType::hoursCountLsb:    return "Hours LSB";
        case MtcMessageType::hoursAndSmpteType: return "Hours/Type MSB";
        default:
            jassertfalse;
            return "Unknown";
    }
}

juce::String getControllerName (int controllerNumber)
{
    if (controllerNumber < 0 || controllerNumber > 127)
        return "Invalid Controller Number";

    const auto& name = controllerNames[controllerNumber];
    if (name.empty ())
        return juce::String (controllerNumber);

    return juce::String (controllerNumber) + " (" + name + ")";
}

juce::String getNoteName (int noteNumber, OctaveType octaveType)
{
    if (noteNumber < 0 || noteNumber > 127)
    {
        return "??";
    }

    const auto name = noteNames[noteNumber % 12];

    const int octave = [&] { return (noteNumber / 12) - (octaveType == OctaveType::Yamaha ? 2 : 1); }();

    return name + juce::String (octave);
}

juce::String getOctaveTypeName (OctaveType octaveType)
{
    switch (octaveType)
    {
        case OctaveType::Yamaha:
            return "Yamaha";
        case OctaveType::Roland:
            return "Roland";
        default:
            jassertfalse; // Should never happen
            return "Unknown Octave Type";
    }
}

juce::String formatValue (uint32_t value, int bitWidth, ValueFormatType formatType, int precision, float formattedMin,
                          float formattedMax, bool suppressPrefix)
{
    jassert (bitWidth >= 1 && bitWidth <= 32);
    const uint32_t maxVal = (bitWidth == 32) ? 0xFFFFFFFFu : (1u << bitWidth) - 1u;

    switch (formatType)
    {
        case ValueFormatType::Decimal:
            return juce::String (value);

        case ValueFormatType::Hex:
        {
            const int hexWidth = (bitWidth + 3) / 4;
            const auto digits  = juce::String::toHexString (static_cast<int> (value)).paddedLeft ('0', hexWidth);
            return suppressPrefix ? digits : juce::String ("0x") + digits;
        }

        case ValueFormatType::Float:
        {
            // some special cases based on the "High Resolution Value User Representations"
            // proposal:
            if (value == maxVal)
                return "MAX";
            else if (value == (1 << bitWidth - 1))
                return "MID";
            else if (value == 0)
                return "MIN";
            const float normalised = static_cast<float> (value) / static_cast<float> (maxVal);
            return juce::String (formattedMin + (formattedMax - formattedMin) * normalised, precision);
        }

        case ValueFormatType::Percent:
        {
            const float normalised = static_cast<float> (value) / static_cast<float> (maxVal);
            return juce::String ((formattedMin + (formattedMax - formattedMin) * normalised) * 100.f, precision) + "%";
        }

        case ValueFormatType::Midi:
        {
            const int fracBits = juce::jmax (0, bitWidth - 7);
            const int intPart  = static_cast<int> (value >> fracBits) + static_cast<int> (formattedMin);

            if (fracBits == 0)
                return juce::String (intPart);

            const uint32_t fracMask = (1u << fracBits) - 1u;
            const float fraction    = static_cast<float> (value & fracMask) / static_cast<float> (1u << fracBits);

            int fracScale = 1;
            for (int i = 0; i < precision; ++i)
                fracScale *= 10;

            const int fracDisplay = static_cast<int> (fraction * static_cast<float> (fracScale));
            return juce::String (intPart) + "." + juce::String (fracDisplay).paddedLeft ('0', precision);
        }

        default:
            jassertfalse;
            return "Unknown Format Type";
    }
}

juce::String formatTime (double seconds)
{
    const int totalMs  = static_cast<int> (seconds * 1000.0f + 0.5f);
    const int ms       = totalMs % 1000;
    const int totalSec = totalMs / 1000;
    const int sec      = totalSec % 60;
    const int totalMin = totalSec / 60;
    const int min      = totalMin % 60;
    const int hours    = totalMin / 60;

    const auto msStr  = juce::String (ms).paddedLeft ('0', 3);
    const auto secStr = juce::String (sec).paddedLeft ('0', 2);

    if (hours > 0)
        return juce::String (hours) + ":" + secStr + "." + msStr;

    if (min > 0)
        return juce::String (min) + ":" + secStr + "." + msStr;

    if (sec >= 10)
        return secStr + "." + msStr;

    return juce::String (sec) + "." + msStr;
}

juce::String formatEndpoint (const Event& e)
{
    return e.endpointName;
}

#if RUN_UNIT_TESTS
#include "test/test_EventNameUtils.inl"
#endif