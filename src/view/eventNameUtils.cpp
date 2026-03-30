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
    "Bank Select",       // 0
    "Modulation Wheel",  // 1
    "Breath Controller", // 2
    "",                  // 3 (Undefined)
    "Foot Controller",   // 4
    "Portamento Time",   // 5
    "Data Entry MSB",    // 6
    "Main Volume",       // 7
    "Balance",           // 8
    "",                  // 9 (Undefined)
    "Pan",               // 10
    "Expression",        // 11
    "Effect Control 1",  // 12
    "Effect Control 2",  // 13
    "",                  // 14
    "",                  // 15 (Undefined)
    "General Purpose 1", // 16
    "General Purpose 2", // 17
    "General Purpose 3", // 18
    "General Purpose 4", // 19
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
    "",                // 85
    "",                // 86
    "",                // 87
    "",                // 88
    "",                // 89
    "",                // 90 (Undefined)
    "Effects 1 Depth", // 91 (Reverb)
    "Effects 2 Depth", // 92 (Tremolo)
    "Effects 3 Depth", // 93 (Chorus)
    "Effects 4 Depth", // 94 (Detune)
    "Effects 5 Depth", // 95 (Phaser)
    "Data Increment",  // 96
    "Data Decrement",  // 97
    "NRPN LSB",        // 98
    "NRPN MSB",        // 99
    "RPN LSB",         // 100
    "RPN MSB",         // 101
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

juce::String getControllerName (int controllerNumber)
{
    if (controllerNumber < 0 || controllerNumber > 127)
        return "Invalid Controller Number";

    const auto& name = controllerNames[controllerNumber];
    if (name.empty())
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
